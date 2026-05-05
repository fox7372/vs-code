#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/* 方便写大数字：8 GiB = 8 * 1024³ */
#define GiB * (1024LL * 1024 * 1024)

int main() {
    /*
     * 用 mmap 申请 8 GiB 虚拟地址空间。
     * MAP_ANONYMOUS     → 不关联文件，纯内存
     * MAP_PRIVATE      → 写时复制
     * 此时只是"画地址空间"，不占物理内存
     */
    volatile uint8_t *p = mmap(
        NULL,
        8 GiB,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1, 0
    );

    printf("mmap: %lx\n", (uintptr_t)p);

    /* mmap 失败返回 MAP_FAILED（即 (void*)-1） */
    if ((intptr_t)p == -1) {
        perror("cannot map");
        exit(1);
    }

    /*
     * 写入时才触发缺页中断（page fault），内核分配物理页面。
     * 整个 8 GiB 只用了 3 个页面（12 KB）的物理内存。
     */
    *(p + 2 GiB) = 1;   /* 触发第 1 次缺页 */
    *(p + 4 GiB) = 2;   /* 触发第 2 次缺页 */
    *(p + 7 GiB) = 3;   /* 触发第 3 次缺页 */

    /* 读之前写过的位置 → 返回写入的值 */
    printf("Read get: %d\n", *(p + 4 GiB));   /* → 2 */
    /*
     * 读没写过的位置 → 返回 0。
     * 因为匿名映射的未写页面共享内核的"零页"，不占物理内存。
     */
    printf("Read get: %d\n", *(p + 6 GiB));   /* → 0 */
    printf("Read get: %d\n", *(p + 7 GiB));   /* → 3 */
}
