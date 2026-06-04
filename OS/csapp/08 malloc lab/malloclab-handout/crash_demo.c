/**
 * crash_demo.c — 演示 handout 的 mm.c 在多线程下崩溃
 *
 * mm.c 声明了 mutex_t mutex，但在 mm_malloc/mm_free 中
 * 从未调用 mutex_lock/mutex_unlock，所以多线程并发操作
 * 空闲链表时会产生 data race → 崩溃或死循环
 *
 * 使用 thread.h 的 spawn() / join() API。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>
#include "mm.h"
#include "memlib.h"


#define SIZE 1000
void worker(void *arg) {
    int tid = (long)arg;
    unsigned int seed = tid;

    printf("[T%d] 启动\n", tid);

    for (int i = 0; i < SIZE; i++) {
        size_t size = (rand_r(&seed) % 64) + 8;
        void *p = mm_malloc(size);
        if (!p) {
            printf("[T%d] mm_malloc 返回 NULL\n", tid);
            return;
        }

        /* 写模式并验证 */
        memset(p, tid, size);
        for (size_t j = 0; j < size; j++) {
            if (((unsigned char *)p)[j] != (unsigned char)tid) {
                printf("[T%d] iter %d: 数据损坏 (offset %zu)\n", tid, i, j);
                return;
            }
        }

        if (rand_r(&seed) % 3 == 0)
            mm_free(p);
    }

    printf("[T%d] 完成\n", tid);
}

int main() {
    setbuf(stdout, NULL);
    printf("mm_segregated.c 多线程测试运行%d\n",SIZE);
    mem_init();
    mm_init();

    spawn(worker);
    spawn(worker);
    join();

    printf("主线程结束\n");
    return 0;
}
