/*
 * qemu-memtrace.c — QEMU TCG Plugin
 *
 * Trace multi-threaded data memory accesses, filtered to RW (writable)
 * regions only. Read-only sections (.text, .rodata) are skipped.
 *
 * RW region detection:
 *   1. ELF PT_LOAD segments with PF_W flag  (parse at startup)
 *   2. mmap/mprotect/munmap/brk syscalls    (runtime tracking)
 *
 * Build (on Linux with qemu-dev installed):
 *   make qemu-memtrace.so
 *
 * Run:
 *   qemu-x86_64 -plugin ./qemu-memtrace.so,elf=./race -- ./race
 *   qemu-aarch64 -plugin ./qemu-memtrace.so,elf=./race -- ./race
 *
 * Output (one access per line):
 *   T<cpu> <R|W> <vaddr> <bytes>
 *
 * Plugin args:
 *   elf=<path>   — guest ELF binary (for initial RW segment detection)
 *   file=<path>  — output file (default: stderr)
 *   nowrite      — suppress write traces, only show reads
 *   noread       — suppress read traces, only show writes
 */

#include <qemu-plugin.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <elf.h>

#include <glib.h>

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

/* ===================================================================
 * RW Page Tracking
 *
 * A hash set of page frame numbers that are known to be writable.
 * Only accesses touching these pages are emitted to the trace.
 * =================================================================== */

#define PAGE_SHIFT 12
#define PAGE_SIZE  (1ul << PAGE_SHIFT)
#define PAGE_NR(a) ((uint64_t)(a) >> PAGE_SHIFT)

static GHashTable *rw_pages;   /* gint64 page_nr → dummy */
static GMutex      rw_lock;

/* Mark [start, start+len) as writable pages */
static void mark_range(uint64_t start, uint64_t len)
{
    if (len == 0) return;
    uint64_t first = PAGE_NR(start);
    uint64_t last  = PAGE_NR(start + len - 1);

    g_mutex_lock(&rw_lock);
    for (uint64_t p = first; p <= last; p++) {
        gint64 *key = g_new(gint64, 1);
        *key = (gint64)p;
        g_hash_table_insert(rw_pages, key, GINT_TO_POINTER(1));
    }
    g_mutex_unlock(&rw_lock);
}

/* Remove [start, start+len) from the writable set */
static void unmark_range(uint64_t start, uint64_t len)
{
    if (len == 0) return;
    uint64_t first = PAGE_NR(start);
    uint64_t last  = PAGE_NR(start + len - 1);

    g_mutex_lock(&rw_lock);
    for (uint64_t p = first; p <= last; p++) {
        gint64 key = (gint64)p;
        g_hash_table_remove(rw_pages, &key);
    }
    g_mutex_unlock(&rw_lock);
}

/* Is the page containing @addr writable? */
static bool is_rw(uint64_t addr)
{
    gint64 key = (gint64)PAGE_NR(addr);
    g_mutex_lock(&rw_lock);
    bool ret = g_hash_table_contains(rw_pages, &key);
    g_mutex_unlock(&rw_lock);
    return ret;
}

/* ===================================================================
 * ELF Parser — extract writable PT_LOAD segments at plugin load
 * =================================================================== */

static void parse_elf_rw(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "memtrace: cannot open ELF '%s'\n", path);
        return;
    }

    Elf64_Ehdr ehdr;
    if (fread(&ehdr, sizeof(ehdr), 1, f) != 1)          goto done;
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0)     goto done;

    size_t phnum = ehdr.e_phnum;
    Elf64_Phdr *ph = malloc(phnum * sizeof(Elf64_Phdr));
    if (!ph) goto done;

    fseek(f, ehdr.e_phoff, SEEK_SET);
    if (fread(ph, sizeof(Elf64_Phdr), phnum, f) != phnum) goto free_ph;

    for (size_t i = 0; i < phnum; i++) {
        if (ph[i].p_type == PT_LOAD && (ph[i].p_flags & PF_W)) {
            fprintf(stderr,
                    "memtrace: RW ELF segment 0x%lx – 0x%lx (%lu bytes)\n",
                    (unsigned long)ph[i].p_vaddr,
                    (unsigned long)(ph[i].p_vaddr + ph[i].p_memsz),
                    (unsigned long)ph[i].p_memsz);
            mark_range(ph[i].p_vaddr, ph[i].p_memsz);
        }
    }

free_ph:
    free(ph);
done:
    fclose(f);
}

/* ===================================================================
 * Syscall Interception — runtime mmap / mprotect / munmap / brk
 *
 * x86_64 syscall numbers: mmap=9  mprotect=10  munmap=11  brk=12
 * aarch64 syscall numbers: mmap=222  mprotect=226  munmap=215  brk=214
 * =================================================================== */

static uint64_t brk_current;   /* tracks brk(0) base */

static void on_syscall(unsigned int cpu, int64_t num,
                        uint64_t a1, uint64_t a2, uint64_t a3,
                        uint64_t a4, uint64_t a5, uint64_t a6,
                        int64_t ret)
{
    switch (num) {
    /* ---- mmap ---- */
    case 9:   /* x86_64 */
    case 222: /* aarch64 */
        if ((int64_t)ret >= 0) {
            if (a3 & 0x2)   /* PROT_WRITE */
                mark_range((uint64_t)ret, a2);
            else
                unmark_range((uint64_t)ret, a2);
        }
        break;

    /* ---- mprotect ---- */
    case 10:  /* x86_64 */
    case 226: /* aarch64 */
        if (ret == 0) {
            if (a3 & 0x2)
                mark_range(a1, a2);
            else
                unmark_range(a1, a2);
        }
        break;

    /* ---- munmap ---- */
    case 11:  /* x86_64 */
    case 215: /* aarch64 */
        if (ret == 0)
            unmark_range(a1, a2);
        break;

    /* ---- brk ---- */
    case 12:  /* x86_64 */
    case 214: /* aarch64 */
        if (a1 == 0) {
            /* brk(0) = query current break */
            brk_current = (uint64_t)ret;
        } else if ((int64_t)ret >= 0) {
            /* heap grew: mark new pages as RW */
            if ((uint64_t)ret > brk_current)
                mark_range(brk_current, (uint64_t)ret - brk_current);
            brk_current = (uint64_t)ret;
        }
        break;
    }
}

/* ===================================================================
 * Memory Access Tracing — the hot path
 * =================================================================== */

static FILE *out;
static bool  trace_reads  = true;
static bool  trace_writes = true;
static GMutex out_lock;
static uint64_t nr_access;

static void on_mem(unsigned int cpu, qemu_plugin_meminfo_t info,
                    uint64_t vaddr, void *udata)
{
    /* --- Filter: only writable pages --- */
    if (!is_rw(vaddr))
        return;

    bool is_store = qemu_plugin_mem_is_store(info);
    if (!trace_reads  && !is_store) return;
    if (!trace_writes &&  is_store) return;

    uint32_t sz = 1u << qemu_plugin_mem_size_shift(info);

    g_mutex_lock(&out_lock);
    fprintf(out, "T%u %c 0x%016" PRIx64 " %u\n",
            cpu, is_store ? 'W' : 'R', vaddr, sz);
    nr_access++;
    g_mutex_unlock(&out_lock);
}

static void on_tb(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    size_t n = qemu_plugin_tb_n_insns(tb);
    for (size_t i = 0; i < n; i++) {
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);
        qemu_plugin_register_vcpu_mem_cb(insn, on_mem,
                                          QEMU_PLUGIN_CB_NO_REGS,
                                          QEMU_PLUGIN_MEM_RW, NULL);
    }
}

/* ===================================================================
 * Plugin install / exit
 * =================================================================== */

static void at_exit(void)
{
    fprintf(stderr, "memtrace: %lu memory accesses traced\n",
            (unsigned long)nr_access);
    if (out != stderr && out != stdout)
        fclose(out);
}

QEMU_PLUGIN_EXPORT
int qemu_plugin_install(qemu_plugin_id_t id,
                        const qemu_info_t *info,
                        int argc, char **argv)
{
    rw_pages = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                      g_free, NULL);
    out = stderr;
    g_mutex_init(&rw_lock);
    g_mutex_init(&out_lock);

    /* Parse plugin arguments */
    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "elf=", 4) == 0) {
            parse_elf_rw(argv[i] + 4);
        } else if (strncmp(argv[i], "file=", 5) == 0) {
            FILE *f = fopen(argv[i] + 5, "w");
            if (f) out = f;
            else fprintf(stderr, "memtrace: cannot open %s\n", argv[i]+5);
        } else if (strcmp(argv[i], "noread") == 0) {
            trace_reads = false;
        } else if (strcmp(argv[i], "nowrite") == 0) {
            trace_writes = false;
        }
    }

    qemu_plugin_register_vcpu_tb_trans_cb(id, on_tb);
    qemu_plugin_register_vcpu_syscall_cb(id, on_syscall);
    atexit(at_exit);

    return 0;
}
