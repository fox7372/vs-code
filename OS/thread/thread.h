#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
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
    thread->entry(thread->id);
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
}

// 析构函数，在程序退出时自动调用 join
__attribute__((destructor)) void cleanup() {
    join();
}

typedef struct 
{
    int iarr[100];
    char carr[100];
    void*ptr[10];
    int typeof_fn;
}data;
typedef struct 
{
    int id ,status;
    pthread_t thread;
    int (*entry)(int arr[]);
    data d;
}thread_r;



void* wrapper_r(void* arg)
{
    thread_r *thr_r = (thread_r *) arg;
    int result = thr_r->entry(thr_r->d.iarr);
    return (void*)(uintptr_t)result;
}

thread_r tpool_r[NTHREAD], *rptr = tpool_r;


void create_r(void *fn,data d)
{
    assert(rptr - tpool_r < NTHREAD);
    *rptr = (thread_r) 
    {
        .id = rptr - tpool_r + 1,
        .status = T_LIVE,
        .entry = fn,
        .d = d,
    };
    pthread_create(&(rptr->thread), NULL, wrapper_r, rptr);
    ++rptr;
}

// ...existing code...

int* join_r()
{   
    int* Data = (int *)malloc(NTHREAD * sizeof(int));
    int index=0;
    void* r = NULL;
    for (int i = 0; i < NTHREAD; i++) {
        thread_r *t = &tpool_r[i];
        if (t->status == T_LIVE) {
            if(pthread_join(t->thread, &r)==0)
            {
                if(r!=NULL)
                {
                     Data[index]=*((int*)r);
                }
            }
            t->status = T_DEAD;
        }
    }
    return Data;
}

