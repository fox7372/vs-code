/*
 * memlib.c - 模拟内存系统的模块。
 *            这样可以让学生的 malloc 包和系统 libc 的 malloc 交替调用而不冲突。
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include<thread.h>
#include<thread-sync.h>
#include "memlib.h"
#include "config.h"
mutex_t mutex1 = MUTEX_INIT();
/* 私有变量 */
static char *mem_start_brk;  /* 指向堆的第一个字节 */
static char *mem_brk;        /* 指向堆的最后一个字节 */
static char *mem_max_addr;   /* 堆的最大合法地址 */

/*
 * mem_init - 初始化内存系统模型（使用 mmap）
 */
void mem_init(void)
{
    /* 用 mmap 分配模拟虚拟内存 */
    mem_start_brk = (char *)mmap(NULL, MAX_HEAP,
                                 PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS,
                                 -1, 0);
    if (mem_start_brk == MAP_FAILED) {
	fprintf(stderr, "mem_init_vm: mmap error\n");
	exit(1);
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* 堆最大合法地址 */
    mem_brk = mem_start_brk;                  /* 堆初始为空 */
}

/*
 * mem_deinit - 释放内存系统模型使用的存储空间
 */
void mem_deinit(void)
{
    munmap(mem_start_brk, MAX_HEAP);
}

/*
 * mem_reset_brk - 重置模拟 brk 指针，清空堆
 */
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/*
 * mem_sbrk - 模拟 sbrk 函数。将堆扩展 incr 字节，返回新区域的起始地址。
 *            在此模型中，堆不能收缩。
 */
void *mem_sbrk(int incr)
{
    char *old_brk;

    mutex_lock(&mutex1);
    old_brk = mem_brk;
    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
	mutex_unlock(&mutex1);
	errno = ENOMEM;
	fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
	return (void *)-1;
    }
    mem_brk += incr;
    mutex_unlock(&mutex1);
    return (void *)old_brk;
}

/*
 * mem_heap_lo - 返回堆第一个字节的地址
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/*
 * mem_heap_hi - 返回堆最后一个字节的地址
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - 返回堆大小（字节）
 */
size_t mem_heapsize()
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - 返回系统页大小
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}
