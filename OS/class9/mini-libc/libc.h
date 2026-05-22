// SPDX-License-Identifier: MIT
// Mini libc — public API (musl-inspired, educational)
//
// Usage: #include "libc.h"  (not <libc.h>)
// Compile with: gcc -ffreestanding -nostdlib -static -fno-stack-protector
//
// In freestanding mode the compiler still provides <stdarg.h>, <stddef.h>, etc.

#ifndef LIBC_H
#define LIBC_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── I/O ──────────────────────────────────────────────────────────────────

int putchar(int c);
int puts(const char *s);
int printf(const char *restrict fmt, ...);
int dprintf(int fd, const char *restrict fmt, ...);

// ── stdlib ───────────────────────────────────────────────────────────────

void _exit(int code) __attribute__((noreturn));
void exit(int code) __attribute__((noreturn));

void *malloc(size_t size);
void  free(void *ptr);

int atoi(const char *s);
long strtol(const char *restrict s, char **restrict endp, int base);

extern char **environ;

// ── string ───────────────────────────────────────────────────────────────

size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strcpy(char *restrict dst, const char *restrict src);
char  *strchr(const char *s, int c);

void  *memset(void *dst, int c, size_t n);
void  *memcpy(void *restrict dst, const void *restrict src, size_t n);
void  *memmove(void *dst, const void *src, size_t n);
int    memcmp(const void *a, const void *b, size_t n);

// ── errno ────────────────────────────────────────────────────────────────

extern int errno;
#define ENOMEM 12

// ── ctype ────────────────────────────────────────────────────────────────

int isupper(int c);
int islower(int c);
int isalpha(int c);
int isdigit(int c);
int isalnum(int c);
int isspace(int c);

// ── internal (exposed for linking) ───────────────────────────────────────

void __libc_start_main(int argc, char **argv, char **envp) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif /* LIBC_H */
