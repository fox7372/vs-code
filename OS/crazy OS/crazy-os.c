// Crazy OS — 一个极简的操作系统模拟器
// 它能同时加载多个 RISC-V 程序，轮转调度执行，并处理系统调用。
//
// 运行在 x86 Linux 上，通过 mini-rv32ima.h 模拟 RISC-V CPU。
// p1.bin / p2.bin 是 RISC-V 机器码，由 crazy-os 加载并模拟执行。

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mini-rv32ima.h"

#define MEM_SIZE   (1 << 20)     // 每个进程 1MB 内存
#define MEM_OFFSET 0x80000000u   // 程序加载的基地址（RISC-V 起始地址）
#define STACK_TOP  (MEM_OFFSET + MEM_SIZE)  // 栈指针初始位置（内存末尾）

// 进程控制块 — 描述一个"进程"的全部状态
struct proc {
    // ===状 RISC-V 虚拟机的态 ===
    struct CPUState cpu;    // CPU 寄存器 + CSR（由 mini-rv32ima.h 定义）
    uint8_t mem[MEM_SIZE];  // 该进程的模拟内存，p1/p2 的代码和数据都在这

    // === Crazy OS 内部状态 ===
    char buf[256];  // 输出缓冲区：攒满一行再打印
    int buf_len;    // 缓冲区已有字符数
};

// 进程初始化：从 .bin 文件加载二进制到模拟内存，设置 CPU 初始状态
static void proc_init(struct proc *p, const char *path) {
    FILE *f = fopen(path, "rb");
    fread(p->mem, 1, MEM_SIZE, f);  // 把文件内容读进进程的内存
    fclose(f);

    p->cpu.mem = p->mem;            // CPU 能访问的内存
    p->cpu.mem_offset = MEM_OFFSET; // 内存起始地址偏移
    p->cpu.mem_size = MEM_SIZE;     // 内存大小
    memset(p->cpu.regs, 0, sizeof(p->cpu.regs));   // 通用寄存器清零
    memset(p->cpu.csrs, 0, sizeof(p->cpu.csrs));   // CSR 清零
    p->cpu.csrs[PC] = MEM_OFFSET;   // PC = 0x80000000（从程序开头执行）
    p->cpu.regs[SP] = STACK_TOP;    // SP = 0x80100000（栈顶）
}

// 系统调用：输出一个字符到终端
// p1/p2 发 ecall 请求输出字符，最终由这个函数真正写到 stdout
static int sys_putchar(struct proc *p, char ch) {
    p->buf[p->buf_len++] = ch;                      // 先攒到缓冲区
    if (ch == '\n' || p->buf_len == sizeof(p->buf) - 1) {  // 遇到换行或满了就刷
        fwrite(p->buf, 1, p->buf_len, stdout);      // 输出到终端
        fflush(stdout);
        p->buf_len = 0;                              // 清空缓冲区
    }
    return 0;
}

// ecall 处理函数 — Crazy OS 的系统调用入口
// p1/p2 执行 ecall 指令后，mini-rv32ima 设置 MCAUSE=8，
// 主循环检测到后调用此函数。
//
// 参数传递约定：
//   a7 = 系统调用号（42 = putchar）
//   a0 = 第一个参数（putchar 的字符）
static void handle_ecall(struct proc *p) {
    int ret = -1;

    switch (p->cpu.regs[A7]) {          // 根据系统调用号分发
        case 42:                        // sys_putchar
            ret = sys_putchar(p, p->cpu.regs[A0]);
            break;
    }

    p->cpu.regs[A0] = ret;              // 返回值写回 a0

    // === 从 M-mode 返回 U-mode（模拟 MRET 指令）===
    // ecall 导致 CPU 从 U-mode 陷入 M-mode，现在需要恢复回去
    uint32_t ms = p->cpu.csrs[MSTATUS];
    uint32_t ef = p->cpu.csrs[EXTRAFLAGS];
    p->cpu.csrs[MSTATUS]    = ((ms & 0x80) >> 4) | ((ef & 3) << 11) | 0x80;
    p->cpu.csrs[EXTRAFLAGS] = (ef & ~3) | ((ms >> 11) & 3);
    p->cpu.csrs[PC]         = p->cpu.csrs[MEPC] + 4;  // 跳到 ecall 的下一条指令
    p->cpu.csrs[MCAUSE]     = 0;                        // 清除异常原因
}

int main(int argc, char *argv[]) {
    int n = argc - 1;                                // 进程数
    struct proc *procs = calloc(n, sizeof(struct proc));  // 分配进程数组
    for (int i = 0; i < n; i++)
        proc_init(&procs[i], argv[i + 1]);           // 依次初始化各进程

    int cur = 0;                                     // 当前运行的进程下标
    while (1) {                                      // 主调度循环
        struct proc *p = &procs[cur];
        rv32ima_step(&p->cpu, 1);                    // 模拟执行一条 RISC-V 指令
        if (p->cpu.csrs[MCAUSE] == 8)                // 如果该指令是 ecall
            handle_ecall(p);                          // 处理系统调用
        cur = (cur + 1) % n;                          // 轮转：切换到下一个进程
    }
}
