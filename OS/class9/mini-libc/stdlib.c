// SPDX-License-Identifier: MIT
// Mini libc — stdlib: _exit, malloc (bump allocator), atoi, strtol

#include "libc.h"

// ── Syscall helpers ──────────────────────────────────────────────────────

#define SYS_brk        12
#define SYS_exit_group 231

static inline long syscall1(long n, long a1) {
    unsigned long ret;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "rcx", "r11", "memory");
    return ret;
}

// ── errno ────────────────────────────────────────────────────────────────

int errno = 0;

// ── environ ──────────────────────────────────────────────────────────────

char **environ = NULL;

// ── _start caller ────────────────────────────────────────────────────────
// Called from crt0.S after _start parses the stack.

int main(int argc, char **argv, char **envp);

void __libc_start_main(int argc, char **argv, char **envp) {
    environ = envp;
    int ret = main(argc, argv, envp);
    exit(ret);
}

// ── exit ─────────────────────────────────────────────────────────────────

void _exit(int code) {
    syscall1(SYS_exit_group, code);
    __builtin_unreachable();
}

void exit(int code) {
    // In a real libc this would call atexit() handlers.
    // For simplicity we skip that and go straight to _exit.
    _exit(code);
}

// ── malloc (bump allocator via brk) ──────────────────────────────────────
//
// This is the simplest possible allocator: just keep a pointer to the
// current "program break" and bump it for each allocation.  free() is a
// no-op — memory grows but is never reclaimed.
//
// musl's actual malloc is a full slab+heap allocator; this is purely
// educational.

#define ALIGN 16
#define round_up(n) (((n) + ALIGN - 1) & ~(ALIGN - 1))

static void *heap_cur = NULL;

void *malloc(size_t size) {
    if (size == 0) return NULL;

    if (!heap_cur) {
        heap_cur = (void *)syscall1(SYS_brk, 0);
    }

    size = round_up(size);
    void *old = heap_cur;
    void *new_brk = (char *)old + size;
    void *actual = (void *)syscall1(SYS_brk, (long)new_brk);

    // brk returns the new program break; if it's less than what we
    // asked for, we ran out of memory.
    if ((long)actual < (long)new_brk)
        return NULL;

    heap_cur = actual;
    return old;
}

void free(void *ptr) {
    (void)ptr;
    // Bump allocator cannot reuse memory.
}

// ── atoi / strtol ────────────────────────────────────────────────────────

int atoi(const char *s) {
    return (int)strtol(s, NULL, 10);
}

long strtol(const char *restrict s, char **restrict endp, int base) {
    const char *p = s;
    long val = 0;
    int neg = 0;

    // skip leading whitespace
    while (*p == ' ' || *p == '\t' || *p == '\n')
        p++;

    // optional sign
    if (*p == '-') { neg = 1; p++; }
    else if (*p == '+') p++;

    // auto-detect base
    if (base == 0) {
        if (*p == '0') {
            p++;
            base = (*p == 'x' || *p == 'X') ? (p++, 16) : 8;
        } else {
            base = 10;
        }
    } else if (base == 16 && *p == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }

    while (1) {
        int d;
        if (*p >= '0' && *p <= '9')       d = *p - '0';
        else if (*p >= 'a' && *p <= 'f')  d = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F')  d = *p - 'A' + 10;
        else break;
        if (d >= base) break;
        val = val * base + d;
        p++;
    }

    if (endp) *endp = (char *)(val == 0 && p == s ? s : p);
    return neg ? -val : val;
}
