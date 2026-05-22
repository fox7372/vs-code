#include <stdio.h>

int main() {
    printf("Trying to disable interrupts in user space\n");

#if defined(__x86_64__) || defined(__i386__)
    asm volatile("cli");
#elif defined(__aarch64__)
    asm volatile("msr daifset, #2");
#else
    #error "Unsupported architecture"
#endif

    printf("This should not be reached\n");
}
