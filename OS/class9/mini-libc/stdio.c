// SPDX-License-Identifier: MIT
// Mini libc — stdio: syscall wrappers + I/O functions

#include "libc.h"

// ── x86-64 Linux syscall helpers ─────────────────────────────────────────
// Syscall ABI: rax=number, rdi,rsi,rdx,r10,r8,r9 = args, return in rax.
// RCX and R11 are always clobbered by the 'syscall' instruction.

#define SYS_write 1

// x86-64 Linux syscall ABI: rax=n, rdi=a1, rsi=a2, rdx=a3, r10=a4, r8=a5, r9=a6
static inline long syscall3(long n, long a1, long a2, long a3) {
    unsigned long ret;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory");
    return ret;
}

// ── I/O ──────────────────────────────────────────────────────────────────

int putchar(int c) {
    char ch = c;
    syscall3(SYS_write, 1, (long)&ch, 1);
    return (unsigned char)ch;
}

int puts(const char *s) {
    while (*s) putchar(*s++);
    putchar('\n');
    return 0;
}

// Internal: print string (for printf)
static void p_str(const char *s) {
    while (*s) putchar(*s++);
}

// Internal: print unsigned long in base b (2-16)
static void p_num(unsigned long n, int b, int upper) {
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char buf[65]; // 64 bits in base-2 + null
    int i = 0;
    if (n == 0) { putchar('0'); return; }
    while (n) {
        buf[i++] = digits[n % b];
        n /= b;
    }
    while (i > 0) putchar(buf[--i]);
}

int printf(const char *restrict fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { putchar(*fmt); continue; }
        fmt++;
        switch (*fmt) {
        case 'l': {
            fmt++;
            switch (*fmt) {
            case 'd': {
                long v = va_arg(ap, long);
                if (v < 0) { putchar('-'); v = -(unsigned long)v; }
                p_num((unsigned long)v, 10, 0);
                break;
            }
            case 'u': {
                unsigned long v = va_arg(ap, unsigned long);
                p_num(v, 10, 0);
                break;
            }
            case 'x': p_num(va_arg(ap, unsigned long), 16, 0); break;
            case 'X': p_num(va_arg(ap, unsigned long), 16, 1); break;
            default:  putchar('%'); putchar('l'); putchar(*fmt); break;
            }
            break;
        }
        case 'd': {
            int v = va_arg(ap, int);
            if (v < 0) { putchar('-'); v = -(unsigned)v; }
            p_num((unsigned long)v, 10, 0);
            break;
        }
        case 'u': {
            unsigned v = va_arg(ap, unsigned);
            p_num(v, 10, 0);
            break;
        }
        case 'x': p_num(va_arg(ap, unsigned), 16, 0); break;
        case 'X': p_num(va_arg(ap, unsigned), 16, 1); break;
        case 'p': {
            void *p = va_arg(ap, void *);
            p_str("0x");
            p_num((unsigned long)p, 16, 0);
            break;
        }
        case 's': p_str(va_arg(ap, const char *)); break;
        case 'c': {
            int ch = va_arg(ap, int);
            putchar(ch);
            break;
        }
        case '%': putchar('%'); break;
        case '\0': putchar('%'); goto done;
        default:  putchar('%'); putchar(*fmt); break;
        }
    }
done:
    va_end(ap);
    return 0;
}

int dprintf(int fd, const char *restrict fmt, ...) {
    // Simple: for dprintf(2, ...), just redirect to stderr fd.
    // For a real impl we'd buffer to a string then write(fd, ...).
    // This is a minimal version that only supports fd=1 or 2.
    (void)fd;
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { putchar(*fmt); continue; }
        fmt++;
        switch (*fmt) {
        case 'd': {
            int v = va_arg(ap, int);
            if (v < 0) { putchar('-'); v = -(unsigned)v; }
            p_num((unsigned long)v, 10, 0);
            break;
        }
        case 's': p_str(va_arg(ap, const char *)); break;
        case 'c': putchar(va_arg(ap, int)); break;
        case '\0': putchar('%'); goto done2;
        default:  putchar('%'); putchar(*fmt); break;
        }
    }
done2:
    va_end(ap);
    return 0;
}
