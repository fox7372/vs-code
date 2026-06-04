#include <thread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NTASKS 6

/* 依赖关系的有向边：src -> dst 表示 src 先执行，dst 后执行 */
struct edge {
    int src, dst;
} edges[] = {
    {0, 1},
    {0, 2},
    {0, 3},
    {1, 4},
    {2, 4},
    {4, 5},
};

/* 任务节点：等待前驱任务完成后才能执行 */
struct task {
    int pending_deps;   /* 尚未满足的前驱依赖数 */
    cond_t cv;          /* 条件变量：等待所有前驱完成 */
} tasks[NTASKS];

// The big, simple lock to protect everyone.
/* 全局大锁，保护所有共享状态 */
mutex_t mutex = MUTEX_INIT();

/* 工作线程：等待依赖清零后执行，再唤醒后继任务 */
void T_worker(int tid) {
    tid--;  /* spawn 传入的 tid 从 1 开始，转为 0-based */

    /* 等待所有前驱任务完成 */
    mutex_lock(&mutex);
    while (!(tasks[tid].pending_deps == 0)) {
        // We can proceed only if all dependencies are cleared.
        cond_wait(&tasks[tid].cv, &mutex);  /* 未就绪则挂起等待 */
    }
    mutex_unlock(&mutex);

    /* 模拟计算 */
    printf("Computing node #%d...\n", tid);
    sleep(1);

    /* 完成计算，通知所有后继任务 */
    mutex_lock(&mutex);
    for (int i = 0; i < LENGTH(edges); i++) {
        struct task *t = &tasks[edges[i].dst];
        if (edges[i].src == tid) {
            // Update all successors of this task
            t->pending_deps--;              /* 减少后继的依赖计数 */
            if (t->pending_deps == 0) {
                // Same as signal: there's at most one waiting
                cond_broadcast(&t->cv);     /* 后继已就绪，唤醒之 */
            }
        }
    }
    mutex_unlock(&mutex);
}

int main() {
    /* 初始化所有任务节点 */
    for (int i = 0; i < NTASKS; i++) {
        tasks[i].pending_deps = 0;
        cond_init(&tasks[i].cv);
    }

    /* 统计每个任务的入度（前驱依赖数） */
    for (int i = 0; i < LENGTH(edges); i++) {
        tasks[edges[i].dst].pending_deps++;
    }

    /* 创建 6 个工作线程并发执行 */
    for (int i = 0; i < NTASKS; i++) {
        spawn(T_worker);
    }

    join();
    printf("Execution completed.\n");
}
