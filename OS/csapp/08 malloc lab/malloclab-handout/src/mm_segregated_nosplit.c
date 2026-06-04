/*
 * mm_segregated_nosplit.c - 分离空闲链表 malloc（无分裂块版本）
 *
 * 设计要点：
 *   1. 分离空闲链表 — 按 2 的幂次划分 20 个大小类别，每个类别独立链表
 *   2. 显式空闲链表 — 空闲块内部存 prev/next 指针（双向链表）
 *   3. 边界标记 — 每个块有 header 和 footer，含 size + allocated 标志
 *   4. 分配策略 — 从目标 class 开始查找，未找到则逐级向更大 class 搜索
 *   5. 无分裂 — 找到满足大小的块即整块分配，不分割剩余空间
 *   6. 合并 — free 时立即合并相邻空闲块，再插入对应 class 的链表
 *
 * 块内存布局（以 32 位为例，WSIZE=4）：
 *
 *   已分配块：      [ header(4B) | ... payload ... | footer(4B) ]
 *   空闲块：        [ header(4B) | prev(4B) | next(4B) | ... | footer(4B) ]
 *
 *   header/footer 格式：PACK(size, alloc_flag)
 *     低 1 位 = allocated 标志
 *     高 31 位 = 块大小（含 header 和 footer）
 *
 * 最小块大小 = 4 words = 16 字节（header + prev + next + footer）
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include<thread.h>
#include<thread-sync.h>
#include "mm.h"
#include "memlib.h"

/* 实验信息 */
team_t team = {
    .teamname = "liu",
    .name1 = "liu",
    .id1 = "2026001",
    .name2 = "",
    .id2 = "",
};

/* ================================================================
 *  常量定义
 * ================================================================ */
#define WSIZE       4
#define PTRSIZE     8
#define DSIZE       8
#define CHUNKSIZE   (1 << 12)
#define CLASS_COUNT 20
#define MIN_BLOCK   (WSIZE + PTRSIZE + PTRSIZE + WSIZE)
mutex_t mutex = MUTEX_INIT();
mutex_t mut_ = MUTEX_INIT();
int arr[CLASS_COUNT];
cond_t cond[CLASS_COUNT];
#define MAX(x, y)   ((x) > (y) ? (x) : (y))

#define PACK(size, alloc)  ((size) | (alloc))
#define GET(p)             (*(unsigned int *)(p))
#define PUT(p, val)        (*(unsigned int *)(p) = (unsigned int)(val))
#define GET_SIZE(p)   (GET(p) & ~0x7)
#define GET_ALLOC(p)  (GET(p) & 0x1)
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define GET_PTR(p)       ((void *)(*(unsigned long *)(p)))
#define PUT_PTR(p, val)  (*(unsigned long *)(p) = (unsigned long)(val))
#define GET_HEAD(num)    GET_PTR(heap_listp + PTRSIZE * (num))
#define GET_PREV(bp)     GET_PTR(bp)
#define GET_NEXT(bp)     GET_PTR((char *)(bp) + PTRSIZE)
#define SET_PREV(bp, val)  PUT_PTR(bp, val)
#define SET_NEXT(bp, val)  PUT_PTR((char *)(bp) + PTRSIZE, val)
#define SET_HEAD(num, val) PUT_PTR(heap_listp + PTRSIZE * (num), val)

static char *heap_listp;

static int search(size_t size) {
    int i;
    for (i = 4; i <= 22; i++) {
        if (size <= (1 << i))
            return i - 4;
    }
    return 22 - 4;
}

void lock_class(int num) {
    mutex_lock(&mutex);
    while (arr[num] != FREE) {
        cond_wait(&cond[num], &mutex);
    }
    arr[num] = LOCKED;
    mutex_unlock(&mutex);
}
void unlock_class(int num) {
    mutex_lock(&mutex);
    arr[num] = FREE;
    cond_broadcast(&cond[num]);
    mutex_unlock(&mutex);
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    bp = (char *)mem_sbrk((int)size);
    if (bp == (void *)-1) return NULL;
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return bp;
}

static void insert(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int num = search(size);
    void *head = GET_HEAD(num);
    SET_NEXT(bp, head);
    if (head) SET_PREV(head, bp);
    SET_PREV(bp, NULL);
    SET_HEAD(num, bp);
}

static void delete(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int num = search(size);
    void *prev = GET_PREV(bp);
    void *next = GET_NEXT(bp);
    if (prev && next) {
        SET_NEXT(prev, next);
        SET_PREV(next, prev);
    } else if (prev && !next) {
        SET_NEXT(prev, NULL);
    } else if (!prev && next) {
        SET_HEAD(num, next);
        SET_PREV(next, NULL);
    } else {
        SET_HEAD(num, NULL);
    }
}

static void *coalesce(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    mutex_lock(&mutex);
    int cls_b = search(GET_SIZE(HDRP(PREV_BLKP(bp))));
    int cls_c = search(GET_SIZE(HDRP(NEXT_BLKP(bp))));

    if (cls_b != cls_c) {
        while (arr[cls_b] != FREE || arr[cls_c] != FREE) {
            if (arr[cls_b] != FREE)
                cond_wait(&cond[cls_b], &mutex);
            else
                cond_wait(&cond[cls_c], &mutex);
        }
        arr[cls_b] = LOCKED;
        arr[cls_c] = LOCKED;
    } else {
        while (arr[cls_b] != FREE)
            cond_wait(&cond[cls_b], &mutex);
        arr[cls_b] = LOCKED;
    }
    mutex_unlock(&mutex);

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    if (prev_alloc && next_alloc) {
        if (cls_b != cls_c) {
            unlock_class(cls_b);
            unlock_class(cls_c);
        } else {
            unlock_class(cls_b);
        }
        lock_class(search(size));
        insert(bp);
        unlock_class(search(size));
        return bp;
    } else if (prev_alloc && !next_alloc) {
        if (cls_b != cls_c) unlock_class(cls_b);
        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        unlock_class(cls_c);
    } else if (!prev_alloc && next_alloc) {
        if (cls_c != cls_b) unlock_class(cls_c);
        delete(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        unlock_class(cls_b);
    } else {
        delete(PREV_BLKP(bp));
        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        if (cls_b != cls_c) unlock_class(cls_b);
        unlock_class(cls_c);
    }
    lock_class(search(GET_SIZE(HDRP(bp))));
    insert(bp);
    unlock_class(search(GET_SIZE(HDRP(bp))));
    return bp;
}

int mm_init(void)
{
    int i;
    if ((heap_listp = (char *)mem_sbrk(CLASS_COUNT * PTRSIZE + 4 * WSIZE)) == (void *)-1)
        return -1;
    for (i = 0; i < CLASS_COUNT; i++) {
        arr[i] = FREE;
        PUT_PTR(heap_listp + i * PTRSIZE, NULL);
        cond_init(&cond[i]);
    }
    PUT(heap_listp + CLASS_COUNT * PTRSIZE, 0);
    PUT(heap_listp + CLASS_COUNT * PTRSIZE + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + CLASS_COUNT * PTRSIZE + 2 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + CLASS_COUNT * PTRSIZE + 3 * WSIZE, PACK(0, 1));
    return 0;
}

void *mm_malloc(size_t size)
{
    size_t asize, extendsize;
    char *bp;
    if (size == 0) return NULL;
    if (size <= DSIZE) asize = 2 * DSIZE;
    else asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    int num = search(asize);
    while (num < CLASS_COUNT) {
        lock_class(num);
        bp = GET_HEAD(num);
        while (bp) {
            if (GET_SIZE(HDRP(bp)) >= asize) {
                delete(bp);
                size_t csize = GET_SIZE(HDRP(bp));
                char *alloc_bp = bp;
                /* 无分裂：整块分配，不分割剩余空间 */
                PUT(HDRP(alloc_bp), PACK(csize, 1));
                PUT(FTRP(alloc_bp), PACK(csize, 1));
                unlock_class(num);
                return alloc_bp;
            }
            bp = GET_NEXT(bp);
        }
        unlock_class(num);
        num++;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) != NULL)
        return bp;
    return NULL;
}

void mm_free(void *ptr)
{
    size_t size;
    if (ptr == NULL) return;
    size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr, *newptr;
    size_t copySize;
    if (size == 0) { mm_free(ptr); return NULL; }
    if (ptr == NULL) return mm_malloc(size);
    newptr = mm_malloc(size);
    if (newptr == NULL) return NULL;
    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize) copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
