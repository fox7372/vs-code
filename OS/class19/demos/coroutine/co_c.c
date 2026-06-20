#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>

#define THREADS  100000      // 协程数量
#define COUNT    10000000    // 调度总次数
#define STACK_SZ 4096        // 每个协程的栈大小

// 协程状态
enum { INIT, YIELD, DONE };

// 协程控制块
struct coroutine {
    jmp_buf ctx;          /* 调用者上下文（main 侧） */
    jmp_buf coro_ctx;     /* 协程自身上下文 */
    char *stack;          // 独立栈空间
    int state;            // 当前状态
    int name;             // 协程编号
    int i;                // 循环计数
    char buf[128];        // 输出缓冲区
};

__attribute__((noinline)) static void coro_entry(struct coroutine *c);

// 分配协程栈（用 volatile 触碰到物理页）
static char *alloc_stack(void) {
    char *mem = malloc(STACK_SZ);
    if (!mem) { perror("malloc"); exit(1); }
    volatile char touch = mem[0];   // 强制分配物理页
    (void)touch;
    return mem;
}

// 初始化一个协程
static void coro_create(struct coroutine *c, int name) {
    c->state = INIT;
    c->name = name;
    c->i = 0;
    c->stack = alloc_stack();

    if (setjmp(c->ctx) == 0) {
        // 切换到协程的独立栈，然后跳转到入口
        void *new_sp = (char *)c->stack + STACK_SZ;
#if defined(__x86_64__)
        __asm__ volatile (
            "mov %0, %%rsp"
            :: "r"(new_sp)
        );
#elif defined(__aarch64__)
        __asm__ volatile (
            "mov sp, %0"
            :: "r"(new_sp)
        );
#else
        /* 其他架构的回退方案：将上下文复制到栈顶 */
        memcpy((char *)c->stack + STACK_SZ - sizeof(jmp_buf), &c->ctx, sizeof(jmp_buf));
#endif
        coro_entry(c);
    }
}

// 协程让出 CPU，将当前缓冲区返回给调用者
__attribute__((noinline))
static const char *coro_yield(struct coroutine *c) {
    c->state = YIELD;
    if (setjmp(c->coro_ctx) == 0) {
        longjmp(c->ctx, 1);     // 跳回 main
    }
    return c->buf;              // 被 resume 后从这里继续
}

// 恢复一个协程的执行
static const char *coro_resume(struct coroutine *c) {
    if (setjmp(c->ctx) == 0) {
        longjmp(c->coro_ctx, 1); // 跳入协程
    }
    return c->buf;
}

// 协程主体：类似 T_worker，不断递增并 yield
static void coro_entry(struct coroutine *c) {
    while (1) {
        c->i++;
        snprintf(c->buf, sizeof(c->buf), "[%d] i = %d", c->name, c->i);
        coro_yield(c);
    }
}

int main(void) {
    struct coroutine *threads = calloc(THREADS, sizeof(struct coroutine));
    if (!threads) { perror("calloc"); return 1; }

    // 创建所有协程
    for (int i = 0; i < THREADS; i++)
        coro_create(&threads[i], i);

    // 随机调度
    srand(42);
    for (int count = 0; count < COUNT; count++) {
        int idx = rand() % THREADS;
        const char *res = coro_resume(&threads[idx]);
        printf("%s\n", res);
    }

    // 清理
    for (int i = 0; i < THREADS; i++)
        free(threads[i].stack);
    free(threads);

    return 0;
}
