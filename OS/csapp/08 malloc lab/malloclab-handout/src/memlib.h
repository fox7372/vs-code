#include <unistd.h>

void mem_init(void);            /* 初始化内存系统 */
void mem_deinit(void);          /* 释放内存系统 */
void *mem_sbrk(int incr);       /* 模拟 sbrk，扩展堆 */
void mem_reset_brk(void);       /* 重置 brk，清空堆 */
void *mem_heap_lo(void);        /* 返回堆起始地址 */
void *mem_heap_hi(void);        /* 返回堆末尾地址 */
size_t mem_heapsize(void);      /* 返回当前堆大小（字节） */
size_t mem_pagesize(void);      /* 返回系统页大小 */
