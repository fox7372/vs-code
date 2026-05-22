#include <setjmp.h>
#include <stdio.h>

/*
 * x86-64 System V ABI 寄存器分类：
 *
 *   调用者保存（caller-saved，函数调用可能修改）：
 *     rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11
 *     -> longjmp 不会恢复它们，longjmp 后仍为 0x514
 *
 *   被调用者保存（callee-saved，函数若使用必须恢复）：
 *     rbx, r12, r13, r14, r15
 *     -> longjmp 将它们恢复为 setjmp 时刻的值 0x114
 *
 * 警告：设置寄存器的 asm 与 longjmp 之间不可有函数调用。
 *       若拆成单独函数，编译器会在函数 epilogue 中自动恢复
 *       callee-saved 寄存器，导致 longjmp 无法体现其恢复效果。
 */

static jmp_buf checkpoint;

int main(void) {
    /* 将所有 GPR 设为 0x114 */
    asm volatile(
        "mov $0x114, %%rax\n\t"
        "mov $0x114, %%rbx\n\t"
        "mov $0x114, %%rcx\n\t"
        "mov $0x114, %%rdx\n\t"
        "mov $0x114, %%rsi\n\t"
        "mov $0x114, %%rdi\n\t"
        "mov $0x114, %%r8\n\t"
        "mov $0x114, %%r9\n\t"
        "mov $0x114, %%r10\n\t"
        "mov $0x114, %%r11\n\t"
        "mov $0x114, %%r12\n\t"
        "mov $0x114, %%r13\n\t"
        "mov $0x114, %%r14\n\t"
        "mov $0x114, %%r15\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory");

    int jmp_rc = setjmp(checkpoint);
    if (jmp_rc == 0) {
        /* 将所有 GPR 设为 0x514 */
        asm volatile(
            "mov $0x514, %%rax\n\t"
            "mov $0x514, %%rbx\n\t"
            "mov $0x514, %%rcx\n\t"
            "mov $0x514, %%rdx\n\t"
            "mov $0x514, %%rsi\n\t"
            "mov $0x514, %%rdi\n\t"
            "mov $0x514, %%r8\n\t"
            "mov $0x514, %%r9\n\t"
            "mov $0x514, %%r10\n\t"
            "mov $0x514, %%r11\n\t"
            "mov $0x514, %%r12\n\t"
            "mov $0x514, %%r13\n\t"
            "mov $0x514, %%r14\n\t"
            "mov $0x514, %%r15\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "memory");
        longjmp(checkpoint, 1);
    }

    puts("Returned via longjmp; inspect registers in debugger.");
    return 0;
}
