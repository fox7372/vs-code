#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#define NTHREAD 6400

enum { T_FREE = 0, T_LIVE, T_DEAD, };

struct thread {
    int id, status;
    pthread_t thread;
    void (*entry)(int);
};

struct thread tpool[NTHREAD], *tptr = tpool;

// 线程入口包装函数
void *wrapper(void *arg) {
    struct thread *thread = (struct thread *)arg;
    thread->entry(thread->id);//调用函数
    return NULL;
}

// 创建线程
void create(void *fn) {
    assert(tptr - tpool < NTHREAD);
    *tptr = (struct thread) {
        .id = tptr - tpool + 1,
        .status = T_LIVE,
        .entry = fn,
    };
    pthread_create(&(tptr->thread), NULL, wrapper, tptr);
    ++tptr;
}

// 等待并回收所有线程
void join() {
    for (int i = 0; i < NTHREAD; i++) {
        struct thread *t = &tpool[i];
        if (t->status == T_LIVE) {
            pthread_join(t->thread, NULL);
            t->status = T_DEAD;
        }
    }
    tptr=tpool;
}

// 析构函数，在程序退出时自动调用 join
__attribute__((destructor)) void cleanup() {
    join();
}


