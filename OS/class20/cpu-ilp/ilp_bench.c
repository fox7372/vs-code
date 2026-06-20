/*
 * ILP benchmark — ARMv7-A / AArch64.
 *
 * Variants:
 *   dep   — 4 dependent integer adds (serial chain, baseline low IPC)
 *   ind   — 4 independent integer adds (dual-issue ALU)
 *   fp    — 4 independent VFP/ASIMD f32 adds (FP pipeline)
 *   neon  — 4 independent NEON q-register adds (4× SIMD per op)
 *   mix   — 2 integer + 2 FP adds interleaved (both pipelines)
 *   imul  — 4 independent integer multiplies (multiply unit)
 *
 * Build:  make
 * Run:    make run
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITERS 100000000

/* ---- dep: fully dependent integer chain ---- */
static void dep(int n) {
#ifdef __aarch64__
    asm volatile(
        "mov w0, #0\n\t"
        "1:\n\t"
        "add w0, w0, #1\n\t"
        "add w0, w0, #1\n\t"
        "add w0, w0, #1\n\t"
        "add w0, w0, #1\n\t"
        "subs %w[n], %w[n], #1\n\t"
        "b.ne 1b\n\t"
        : [n] "+r"(n)
        :
        : "w0", "cc"
    );
#else
    asm volatile(
        "mov r0, #0\n\t"
        "1:\n\t"
        "add r0, r0, #1\n\t"
        "add r0, r0, #1\n\t"
        "add r0, r0, #1\n\t"
        "add r0, r0, #1\n\t"
        "subs %[n], %[n], #1\n\t"
        "bne 1b\n\t"
        : [n] "+r"(n)
        :
        : "r0", "cc"
    );
#endif
}

/* ---- ind: 4 independent integer adds ---- */
static void ind(int n) {
#ifdef __aarch64__
    asm volatile(
        "mov w0, #0\n\t"
        "mov w1, #0\n\t"
        "mov w2, #0\n\t"
        "mov w3, #0\n\t"
        "1:\n\t"
        "add w0, w0, #1\n\t"
        "add w1, w1, #1\n\t"
        "add w2, w2, #1\n\t"
        "add w3, w3, #1\n\t"
        "subs %w[n], %w[n], #1\n\t"
        "b.ne 1b\n\t"
        : [n] "+r"(n)
        :
        : "w0", "w1", "w2", "w3", "cc"
    );
#else
    asm volatile(
        "mov r0, #0\n\t"
        "mov r1, #0\n\t"
        "mov r2, #0\n\t"
        "mov r3, #0\n\t"
        "1:\n\t"
        "add r0, r0, #1\n\t"
        "add r1, r1, #1\n\t"
        "add r2, r2, #1\n\t"
        "add r3, r3, #1\n\t"
        "subs %[n], %[n], #1\n\t"
        "bne 1b\n\t"
        : [n] "+r"(n)
        :
        : "r0", "r1", "r2", "r3", "cc"
    );
#endif
}

/* ---- fp: 4 independent VFP f32 adds ---- */
static void fp(int n) {
    float c[8] __attribute__((aligned(8))) =
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    /*
     * s0 += s1,  s2 += s3,  s4 += s5,  s6 += s7
     * 4 independent accumulation chains through the FP pipeline.
     */
#ifdef __aarch64__
    asm volatile(
        "ldr s0, [%[p], #0]\n\t"
        "ldr s1, [%[p], #4]\n\t"
        "ldr s2, [%[p], #8]\n\t"
        "ldr s3, [%[p], #12]\n\t"
        "ldr s4, [%[p], #16]\n\t"
        "ldr s5, [%[p], #20]\n\t"
        "ldr s6, [%[p], #24]\n\t"
        "ldr s7, [%[p], #28]\n\t"
        "1:\n\t"
        "fadd s0, s0, s1\n\t"
        "fadd s2, s2, s3\n\t"
        "fadd s4, s4, s5\n\t"
        "fadd s6, s6, s7\n\t"
        "subs %w[n], %w[n], #1\n\t"
        "b.ne 1b\n\t"
        : [n] "+r"(n)
        : [p] "r"(c)
        : "v0", "v1", "cc"
    );
#else
    asm volatile(
        "vldr s0, [%[p], #0]\n\t"
        "vldr s1, [%[p], #4]\n\t"
        "vldr s2, [%[p], #8]\n\t"
        "vldr s3, [%[p], #12]\n\t"
        "vldr s4, [%[p], #16]\n\t"
        "vldr s5, [%[p], #20]\n\t"
        "vldr s6, [%[p], #24]\n\t"
        "vldr s7, [%[p], #28]\n\t"
        "1:\n\t"
        "vadd.f32 s0, s0, s1\n\t"
        "vadd.f32 s2, s2, s3\n\t"
        "vadd.f32 s4, s4, s5\n\t"
        "vadd.f32 s6, s6, s7\n\t"
        "subs %[n], %[n], #1\n\t"
        "bne 1b\n\t"
        : [n] "+r"(n)
        : [p] "r"(c)
        : "d0", "d1", "d2", "d3", "cc"
    );
#endif
}

/* ---- neon: 4 independent NEON q-register adds (4 × int32 per op) ---- */
static void neon(int n) {
    /*
     * Each vadd.i32 / add vN.4s does 4 parallel int32 adds.
     * 4 such ops per iter → 16 int32 adds logically.
     * q0 += q1, q2 += q3, q4 += q5, q6 += q7  — all independent.
     */
#ifdef __aarch64__
    asm volatile(
        "movi v0.4s, #1\n\t"
        "movi v1.4s, #1\n\t"
        "movi v2.4s, #1\n\t"
        "movi v3.4s, #1\n\t"
        "movi v4.4s, #1\n\t"
        "movi v5.4s, #1\n\t"
        "movi v6.4s, #1\n\t"
        "movi v7.4s, #1\n\t"
        "1:\n\t"
        "add v0.4s, v0.4s, v1.4s\n\t"
        "add v2.4s, v2.4s, v3.4s\n\t"
        "add v4.4s, v4.4s, v5.4s\n\t"
        "add v6.4s, v6.4s, v7.4s\n\t"
        "subs %w[n], %w[n], #1\n\t"
        "b.ne 1b\n\t"
        : [n] "+r"(n)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "cc"
    );
#else
    asm volatile(
        "vmov.i32 q0, #1\n\t"
        "vmov.i32 q1, #1\n\t"
        "vmov.i32 q2, #1\n\t"
        "vmov.i32 q3, #1\n\t"
        "vmov.i32 q4, #1\n\t"
        "vmov.i32 q5, #1\n\t"
        "vmov.i32 q6, #1\n\t"
        "vmov.i32 q7, #1\n\t"
        "1:\n\t"
        "vadd.i32 q0, q0, q1\n\t"
        "vadd.i32 q2, q2, q3\n\t"
        "vadd.i32 q4, q4, q5\n\t"
        "vadd.i32 q6, q6, q7\n\t"
        "subs %[n], %[n], #1\n\t"
        "bne 1b\n\t"
        : [n] "+r"(n)
        :
        : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "cc"
    );
#endif
}

/* ---- mix: 2 integer + 2 FP adds, interleaved ---- */
static void mix(int n) {
    float c[4] __attribute__((aligned(8))) = {1.0f, 1.0f, 1.0f, 1.0f};
    /*
     * add (integer ALU) and fadd (FP/SIMD pipeline) can execute
     * in parallel because they use different execution units.
     * w0 += 1  ‖  s0 += s4
     * w1 += 1  ‖  s2 += s5
     */
#ifdef __aarch64__
    asm volatile(
        "mov w0, #0\n\t"
        "mov w1, #0\n\t"
        "eor v0.16b, v0.16b, v0.16b\n\t"
        "ldr s4, [%[p], #0]\n\t"
        "ldr s5, [%[p], #4]\n\t"
        "1:\n\t"
        "add w0, w0, #1\n\t"
        "fadd s0, s0, s4\n\t"
        "add w1, w1, #1\n\t"
        "fadd s2, s2, s5\n\t"
        "subs %w[n], %w[n], #1\n\t"
        "b.ne 1b\n\t"
        : [n] "+r"(n)
        : [p] "r"(c)
        : "w0", "w1", "v0", "v1", "cc"
    );
#else
    asm volatile(
        "mov r0, #0\n\t"
        "mov r1, #0\n\t"
        "veor d0, d0, d0\n\t"
        "veor d1, d1, d1\n\t"
        "vldr s4, [%[p], #0]\n\t"
        "vldr s5, [%[p], #4]\n\t"
        "1:\n\t"
        "add r0, r0, #1\n\t"
        "vadd.f32 s0, s0, s4\n\t"
        "add r1, r1, #1\n\t"
        "vadd.f32 s2, s2, s5\n\t"
        "subs %[n], %[n], #1\n\t"
        "bne 1b\n\t"
        : [n] "+r"(n)
        : [p] "r"(c)
        : "r0", "r1", "d0", "d1", "d2", "cc"
    );
#endif
}

/* ---- imul: 4 independent integer multiplies ---- */
static void imul(int n) {
#ifdef __aarch64__
    asm volatile(
        "mov w0, #1\n\t"
        "mov w1, #1\n\t"
        "mov w2, #1\n\t"
        "mov w3, #1\n\t"
        "mov w4, #3\n\t"
        "1:\n\t"
        "mul w0, w0, w4\n\t"
        "mul w1, w1, w4\n\t"
        "mul w2, w2, w4\n\t"
        "mul w3, w3, w4\n\t"
        "subs %w[n], %w[n], #1\n\t"
        "b.ne 1b\n\t"
        : [n] "+r"(n)
        :
        : "w0", "w1", "w2", "w3", "w4", "cc"
    );
#else
    asm volatile(
        "mov r0, #1\n\t"
        "mov r1, #1\n\t"
        "mov r2, #1\n\t"
        "mov r3, #1\n\t"
        "mov r4, #3\n\t"
        "1:\n\t"
        "mul r0, r0, r4\n\t"
        "mul r1, r1, r4\n\t"
        "mul r2, r2, r4\n\t"
        "mul r3, r3, r4\n\t"
        "subs %[n], %[n], #1\n\t"
        "bne 1b\n\t"
        : [n] "+r"(n)
        :
        : "r0", "r1", "r2", "r3", "r4", "cc"
    );
#endif
}

/* ---- dispatcher ---- */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s <dep|ind|fp|neon|mix|imul>\n"
            "  dep   dependent integer adds   (low ILP baseline)\n"
            "  ind   independent integer adds (dual-issue ALU)\n"
            "  fp    independent VFP f32 adds (FP pipeline)\n"
            "  neon  NEON q-register adds     (4x SIMD lanes)\n"
            "  mix   integer + FP interleaved (both pipelines)\n"
            "  imul  independent multiplies   (multiply unit)\n",
            argv[0]);
        return 1;
    }

    int n = ITERS;
    const char *v = argv[1];

    if      (!strcmp(v, "dep"))  dep(n);
    else if (!strcmp(v, "ind"))  ind(n);
    else if (!strcmp(v, "fp"))   fp(n);
    else if (!strcmp(v, "neon")) neon(n);
    else if (!strcmp(v, "mix"))  mix(n);
    else if (!strcmp(v, "imul")) imul(n);
    else { fprintf(stderr, "Unknown variant: %s\n", v); return 1; }

    return 0;
}
