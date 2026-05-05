// Copyright 2024 Hanzhi Liu (MIT Licenced)
//
// mini-rv32ima 的主程序
// 加载 RISC-V 二进制文件并驱动模拟器执行
// 同时负责解析命令行参数（作为 main 函数的参数传入模拟程序）

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "mini-rv32ima.h"

// ============================================================
// 内存布局配置
// ============================================================
#define RAM_SIZE (64*1024*1024) // 默认内存大小为 64MB
#define RAM_TEXT_START   0       // 程序代码起始地址
#define RAM_TEXT_END     RAM_STACK_START  // 代码区结束地址
#define RAM_STACK_START (RAM_SIZE/2)      // 栈区起始地址（内存中间位置）
#define RAM_STACK_END    RAM_SIZE         // 栈区结束地址（内存末尾）

// ============================================================
// DumpState — 打印 CPU 的完整寄存器状态
// ============================================================
// 输出当前 PC（程序计数器）、当前指令编码以及所有 32 个通用寄存器的值。
void DumpState(struct CPUState * core) {
    uint32_t pc = core->csrs[PC];
    uint32_t pc_offset = pc - RAM_TEXT_START;
    uint32_t ir = 0;
    printf(" PC:%08x", pc);
    if (pc_offset >= 0 && pc_offset < RAM_TEXT_END - 3) {
        ir = *((uint32_t*)(&((uint8_t*)core->mem)[pc_offset]));
        printf(" [%08x]", ir);
    } else {
        printf(" [xxxxxxxx]");
    }
    printf("\n");
    uint32_t * regs = core->regs;
    printf("  Z:%08x  ra:%08x  sp:%08x  gp:%08x\n",  regs[Z], regs[RA], regs[SP], regs[GP]);
    printf(" tp:%08x  t0:%08x  t1:%08x  t2:%08x\n", regs[TP], regs[T0], regs[T1], regs[T2]);
    printf(" s0:%08x  s1:%08x  a0:%08x  a1:%08x\n", regs[S0], regs[S1], regs[A0], regs[A1]);
    printf(" a2:%08x  a3:%08x  a4:%08x  a5:%08x\n", regs[A2], regs[A3], regs[A4], regs[A5]);
    printf(" a6:%08x  a7:%08x  s2:%08x  s3:%08x\n", regs[A6], regs[A7], regs[S2], regs[S3]);
    printf(" s4:%08x  s5:%08x  s6:%08x  s7:%08x\n", regs[S4], regs[S5], regs[S6], regs[S7]);
    printf(" s8:%08x  s9:%08x s10:%08x s11:%08x\n", regs[S8], regs[S9], regs[S10], regs[S11]);
    printf(" t3:%08x  t4:%08x  t5:%08x  t6:%08x\n", regs[T3], regs[T4], regs[T5], regs[T6]);
    printf("\n");
}

// ============================================================
// xtoi — 将十六进制字符串转换为整数
// ============================================================
// 字符串格式以 "0x" 或 "0X" 开头，例如 "0x1234"。
int xtoi(char * s) {
    int res = 0;
    for (int i = 2; s[i]; i ++) {
        res *= 16;
        if (s[i] >= 'a' && s[i] <= 'f') {
            res =+ s[i] - 'a';
        } else {
            res += s[i] - '0';
        }
    }
    return res;
}

// ============================================================
// main — 主入口
// ============================================================
// 用法: ./mini-rv32ima <二进制文件路径> [参数1] [参数2] ...
//
// 功能：
//   1. 分配并初始化模拟内存
//   2. 加载 RISC-V 二进制文件到内存起始位置
//   3. 解析命令行参数作为模拟程序的 main 函数参数
//   4. 在栈上布置参数数组，设置 a0=argc, a1=argv, sp=栈指针
//   5. 循环调用 rv32ima_step 执行指令，直至程序退出
int main(int argc, char ** argv) {
    // ----------------------------------------------------------
    // 1. 参数检查
    // ----------------------------------------------------------
    if (argc < 2) {
        printf("用法: ./mini-rv32ima <测试文件路径> <参数1> <参数2> ... <参数n>\n");
        printf("- 测试文件应为 RV32I 二进制，指令从偏移 0 开始加载\n");
        printf("- 为简化，仅支持十进制/十六进制整数类型的 main 参数\n");
        return 0;
    }
    char * image_filename = argv[1];
    printf("[mini-rv32ima] 加载二进制文件: %s\n", image_filename);

    // ----------------------------------------------------------
    // 2. 分配并初始化 CPU 状态和内存
    // ----------------------------------------------------------
    struct CPUState state;
    printf("[mini-rv32ima] 分配内存大小 = %#x\n", RAM_SIZE);
    memset(&state, 0, sizeof(state));
    state.mem = malloc(RAM_SIZE);
    if (!state.mem) {
        fprintf(stderr, "错误: 无法分配模拟内存。\n");
        return 1;
    }
    state.mem_size = RAM_SIZE;
    memset(state.mem, 0, state.mem_size);
    state.mem_offset = RAM_TEXT_START;
    state.csrs[PC] = state.mem_offset;  // PC 从内存起始位置开始

    // ----------------------------------------------------------
    // 3. 加载二进制文件到内存
    // ----------------------------------------------------------
    FILE * f = fopen(image_filename, "rb");
    if (!f || ferror(f)) {
        fprintf(stderr, "错误: 无法打开二进制文件 \"%s\"\n", image_filename);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (flen > RAM_TEXT_END - RAM_TEXT_START) {
        fprintf(stderr, "错误: 二进制文件过大 (%#lx 字节)\n", flen);
        fclose(f);
        return 1;
    }
    if (fread(state.mem + RAM_TEXT_START, flen, 1, f) != 1) {
        fprintf(stderr, "错误: 加载二进制文件失败\n");
        return 1;
    }
    fclose(f);

    // ----------------------------------------------------------
    // 4. 解析 main 函数参数
    // ----------------------------------------------------------
    // 将命令行剩余参数转换为整数数组，放置在栈顶
    #define MAX_MAINARGS 4
    if (argc > 2 + MAX_MAINARGS) {
        printf("[mini-rv32ima] 警告: main 参数应 <= %d，超出部分将被丢弃\n", MAX_MAINARGS);
    }
    int margc = (argc > 2 + MAX_MAINARGS ? MAX_MAINARGS : argc - 2);
    int margs[MAX_MAINARGS] = {0};
    printf("[mini-rv32ima] main 参数:\n");
    for (int i = 0; i < margc; i ++) {
        if (argv[2 + i][0] == '0' && argv[2 + i][1] == 'x') {
            margs[i] = xtoi(argv[2 + i]);  // 十六进制解析
        } else {
            margs[i] = atoi(argv[2 + i]);  // 十进制解析
        }
        printf("- %d\n", margs[i]);
    }
    // 在栈顶放置参数数组，并设置 a0=argc, a1=argv, sp=栈指针
    uint32_t sp = RAM_STACK_END - margc * sizeof(int32_t);
    memcpy(state.mem + sp, margs, RAM_STACK_END - sp);
    state.regs[SP] = sp;
    state.regs[A0] = margc;
    state.regs[A1] = sp;

    // ----------------------------------------------------------
    // 5. 执行模拟
    // ----------------------------------------------------------
    printf("初始状态:\n");
    DumpState(&state);
    int ret;
    int debug_climit = 10000;  // 调试用：最大执行指令数限制
    do {
        ret = rv32ima_step(&state, 1);
        if (ret != 0) printf("minirv32ima 返回 ret=%d !=0\n", ret);
        if (--debug_climit <= 0) {
            fprintf(stderr, "错误: 超过调试指令数限制\n");
            break;
        }
    } while (ret == 0 && state.csrs[PC] != 0);
    printf("最终状态:\n");
    DumpState(&state);

    // ----------------------------------------------------------
    // 6. 返回
    // ----------------------------------------------------------
    return 0;
}
