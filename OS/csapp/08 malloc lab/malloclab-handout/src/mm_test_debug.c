#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include<thread.h>
#include<thread-sync.h>
#include "mm.h"
#include "memlib.h"

team_t team = {
    .teamname = "liu",
    .name1 = "liu",
    .id1 = "2026001",
    .name2 = "",
    .id2 = "",
};

#define WSIZE       4
#define PTRSIZE     8
#define DSIZE       8
#define CHUNKSIZE   (1 << 12)
#define CLASS_COUNT 20
#define MIN_BLOCK   (WSIZE + PTRSIZE + PTRSIZE + WSIZE)

mutex_t mutex = MUTEX_INIT();
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
    while(arr[num] != FREE)
    {
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

static void insert(void *bp);
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    bp = (char *)mem_sbrk((int)size);
    if (bp == (void *)-1) return NULL;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    lock_class(search(size));
    insert(bp);
    return bp;
}

static void insert(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int num = search(size);
    void *head = GET_HEAD(num);
    SET_NEXT(bp, head);
    if (head) SET_PREV(head, bp);
    SET_PREV(bp, NULL);
    SET_HEAD(num, bp);
}

static void delete(void *bp) {
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

void *find_fit(size_t asize) {
    int num = search(asize);
    void *bp;
    while (num < CLASS_COUNT)
    {
        lock_class(num);
        bp = GET_HEAD(num);
        while (bp) {
            if (GET_SIZE(HDRP(bp)) >= asize) {
                return bp;
            }
            bp = GET_NEXT(bp);
        }
        unlock_class(num);
        num++;
    }
    return NULL;
}

static void place(void *bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));
    int cls = search(csize);
    fprintf(stderr,"P lock %d\n",cls);fflush(stderr);lock_class(cls);
    delete(bp);
    unfprintf(stderr,"P lock %d\n",cls);fflush(stderr);lock_class(cls);
    if ((csize - asize) >= MIN_BLOCK) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        fprintf(stderr,"P rlock %d\n",search(csize-asize));fflush(stderr);lock_class(search(csize - asize));
        insert(bp);
        unfprintf(stderr,"P rlock %d\n",search(csize-asize));fflush(stderr);lock_class(search(csize - asize));
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc) {
        lock_class(search(size));
        insert(bp);
        unlock_class(search(size));
        return bp;
    }
    else if (prev_alloc && !next_alloc) {
        int cls_a = search(size);
        int cls_b = search(GET_SIZE(HDRP(NEXT_BLKP(bp))));
        mutex_lock(&mutex);
        while(arr[cls_b] != FREE) {
            cond_wait(&cond[cls_b], &mutex);
        }
        arr[cls_b] = LOCKED;
        mutex_unlock(&mutex);
        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        arr[cls_b] = FREE;
        cond_broadcast(&cond[cls_a]);
        cond_broadcast(&cond[cls_b]);
    }
    else if (!prev_alloc && next_alloc) {
        int cls_a = search(size);
        int cls_b = search(GET_SIZE(HDRP(PREV_BLKP(bp))));
        mutex_lock(&mutex);
        while(arr[cls_b] != FREE) {
            cond_wait(&cond[cls_b], &mutex);
        }
        arr[cls_b] = LOCKED;
        mutex_unlock(&mutex);
        delete(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        arr[cls_b] = FREE;
        cond_broadcast(&cond[cls_b]);
    }
    else {
        int cls_a = search(size);
        int cls_b = search(GET_SIZE(HDRP(PREV_BLKP(bp))));
        int cls_c = search(GET_SIZE(HDRP(NEXT_BLKP(bp))));
        mutex_lock(&mutex);
        while(arr[cls_b] != FREE || arr[cls_c] != FREE) {
            if(arr[cls_b] != FREE)
                cond_wait(&cond[cls_b], &mutex);
            else
                cond_wait(&cond[cls_c], &mutex);
        }
        arr[cls_b] = LOCKED;
        arr[cls_c] = LOCKED;
        mutex_unlock(&mutex);
        delete(PREV_BLKP(bp));
        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        arr[cls_b] = FREE;
        arr[cls_c] = FREE;
        cond_broadcast(&cond[cls_b]);
        cond_broadcast(&cond[cls_c]);
    }
    lock_class(search(GET_SIZE(HDRP(bp))));
    insert(bp);
    unlock_class(search(GET_SIZE(HDRP(bp))));
    return bp;
}

int mm_init(void) {
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
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

void *mm_malloc(size_t size) {
    size_t asize, extendsize;
    char *bp;
    if (size == 0) return NULL;
    if (size <= DSIZE) asize = 2 * DSIZE;
    else asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

void mm_free(void *ptr) {
    size_t size;
    if (ptr == NULL) return;
    size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

void *mm_realloc(void *ptr, size_t size) {
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
