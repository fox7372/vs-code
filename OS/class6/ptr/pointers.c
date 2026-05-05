#include <stdio.h>
#include <stdint.h>

/* ========== 全局变量 —— 存在数据段 (.data) ========== */
int global_var = 0x12345678;

/* ========== 常量 —— 存在只读数据段 (.rodata) ========== */
const int const_var = 0xabcdef12;

int main() {
    /* ========== 局部变量 —— 存在栈上 ========== */
    int stack_var1 = 0xdeadbeef;
    int stack_var2 = 0xcafebabe;

    /* ——— 打印各变量的地址 ——— */
    printf("=== 内存地址演示 ===\n\n");

    /* 1. 代码段（.text）—— main 函数所在的位置 */
    printf("1. 代码段（Text Segment）:\n");
    printf("   main 函数地址  : %p\n", (void*)main);

    /* 2. 数据段（.data）—— 已初始化的全局变量 */
    printf("\n2. 数据段（Data Segment）:\n");
    printf("   global_var 地址     : %p\n", (void*)&global_var);
    printf("   global_var 值       : 0x%x\n", global_var);

    /* 3. 只读数据段（.rodata）—— 常量 */
    printf("\n3. 只读数据段（Read-only Data Segment）:\n");
    printf("   const_var 地址      : %p\n", (void*)&const_var);
    printf("   const_var 值        : 0x%x\n", const_var);

    /* 4. 栈段 —— 局部变量 */
    printf("\n4. 栈段（Stack Segment）:\n");
    printf("   stack_var1 地址     : %p\n", (void*)&stack_var1);
    printf("   stack_var1 值       : 0x%x\n", stack_var1);
    printf("   stack_var2 地址     : %p\n", (void*)&stack_var2);
    printf("   stack_var2 值       : 0x%x\n", stack_var2);
    printf("   注意：后声明变量地址更高，说明栈是向下生长的\n");

    /* ——— 验证 main 地址处确实存放的是二进制机器码 ——— */
    printf("\n=== 验证 main 函数的二进制代码 ===\n");

    /* 将函数指针转为 uint8_t*，按字节读取 */
    uint8_t *main_code = (uint8_t*)main;

    printf("main() 函数前 20 字节（十六进制）:\n");
    for (int i = 0; i < 20; i++) {
        printf("0x%02x ", main_code[i]);
        if ((i + 1) % 8 == 0)
            printf("\n");
    }
    if (20 % 8 != 0)
        printf("\n");

    return 0;
}
