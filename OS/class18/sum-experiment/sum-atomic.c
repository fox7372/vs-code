extern long sum, N;

void T_sum() {
    for (int i = 0; i < N; i++) {
#ifdef __x86_64__
        asm volatile(
            "lock addq $1, %0" : "+m"(sum)
        );
#elif defined(__aarch64__)
        asm volatile(
            "1: ldaxr x0, [%0]\n"
            "   add  x0, x0, #1\n"
            "   stlxr w1, x0, [%0]\n"
            "   cbnz w1, 1b"
            :
            : "r"(&sum)
            : "x0", "w1", "memory"
        );
#endif
    }
}
