#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "fat32.h"

struct fat32hdr *hdr;

void *mmap_disk(const char *fname);
void dfs_scan(u32 clusId, int depth);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
        exit(1);
    }

    setbuf(stdout, NULL);

    assert(sizeof(struct fat32hdr) == 512);
    assert(sizeof(struct fat32dent) == 32);

    // Map disk image to memory.
    // The file system is a in-memory data structure now.
    hdr = mmap_disk(argv[1]);

    // File system traversal.
    dfs_scan(hdr->BPB_RootClus, 0);

    munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
}

void *mmap_disk(const char *fname) {
    int fd = open(fname, O_RDWR);

    if (fd < 0) {
        goto release;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        goto release;
    }

    struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (hdr == MAP_FAILED) {
        goto release;
    }

    close(fd);

    assert(hdr->Signature_word == 0xaa55); // this is an MBR
    assert(hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec == size);

    printf("%s: FAT32 filesystem image\n", fname);
    printf("  Bytes/Sector     : %d\n", hdr->BPB_BytsPerSec);
    printf("  Sectors/Cluster  : %d (%d KiB/cluster)\n",
           hdr->BPB_SecPerClus,
           hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus / 1024);
    printf("  Reserved Sectors : %d\n", hdr->BPB_RsvdSecCnt);
    printf("  Number of FATs   : %d\n", hdr->BPB_NumFATs);
    printf("  Sectors per FAT  : %d\n", hdr->BPB_FATSz32);
    printf("  Total Sectors    : %d (%d MiB)\n",
           hdr->BPB_TotSec32,
           hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec / 1024 / 1024);
    printf("  Root Cluster     : %d\n", hdr->BPB_RootClus);
    printf("  Serial Number    : %08x\n", hdr->BS_VolID);
    printf("  Volume Label     : \"%.11s\"\n", hdr->BS_VolLab);
    printf("  FS Type          : \"%.8s\"\n", hdr->BS_FilSysType);
    printf("\n");
    return hdr;

release:
    perror("map disk");
    if (fd > 0) {
        close(fd);
    }
    exit(1);
}

u32 next_cluster(int n) {
    // RTFM: Sec 4.1

    u32 off = hdr->BPB_RsvdSecCnt * hdr->BPB_BytsPerSec;
    u32 *fat = (u32 *)((u8 *)hdr + off);
    return fat[n];
}

void *cluster_to_sec(int n) {
    // RTFM: Sec 3.5 and 4 (TRICKY)
    // Don't copy code. Write your own.

    u32 DataSec = hdr->BPB_RsvdSecCnt + hdr->BPB_NumFATs * hdr->BPB_FATSz32;
    DataSec += (n - 2) * hdr->BPB_SecPerClus;
    return ((char *)hdr) + DataSec * hdr->BPB_BytsPerSec;
}

void get_filename(struct fat32dent *dent, char *buf) {
    // RTFM: Sec 6.1

    int len = 0;
    for (int i = 0; i < sizeof(dent->DIR_Name); i++) {
        if (dent->DIR_Name[i] != ' ') {
            if (i == 8)
                buf[len++] = '.';
            buf[len++] = dent->DIR_Name[i];
        }
    }
    buf[len] = '\0';
}

void dfs_scan(u32 clusId, int depth) {
    // RTFM: Sec 6

    for (; clusId < CLUS_INVALID; clusId = next_cluster(clusId)) {
        int ndents = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus / sizeof(struct fat32dent);

        for (int d = 0; d < ndents; d++) {
            struct fat32dent *dent = (struct fat32dent *)cluster_to_sec(clusId) + d;
            if (dent->DIR_Name[0] == 0x00 ||
                dent->DIR_Name[0] == 0xe5 ||
                dent->DIR_Attr & (ATTR_HIDDEN | ATTR_VOLUME_ID))
                continue;

            char fname[32];
            get_filename(dent, fname);

            for (int i = 0; i < 4 * depth; i++)
                putchar(' ');

            u32 dataClus = dent->DIR_FstClusLO | (dent->DIR_FstClusHI << 16);
            if (dent->DIR_Attr & ATTR_DIRECTORY) {
                printf("[DIR  %-14s] clus=#%-6d\n", fname, dataClus);
                if (dent->DIR_Name[0] != '.') {
                    dfs_scan(dataClus, depth + 1);
                }
            } else {
                printf("[%-18s] %7.1lf KiB  ", fname, dent->DIR_FileSize / 1024.0);
                if (dataClus == 0) {
                    printf("(empty file)\n");
                } else {
                    // Walk FAT chain and print compressed cluster ranges
                    int nclusters = 0;
                    u32 range_first = dataClus, range_prev = dataClus;
                    for (u32 c = dataClus; c < CLUS_INVALID; ) {
                        if (c != range_prev + 1 || nclusters == 0) {
                            if (nclusters > 0 && range_prev != range_first)
                                printf("-%d", range_prev);
                            if (nclusters > 0) printf(", ");
                            printf("#%d", c);
                            range_first = c;
                        }
                        range_prev = c;
                        nclusters++;
                        c = next_cluster(c);
                    }
                    if (nclusters > 1 && range_prev != range_first)
                        printf("-%d", range_prev);
                    printf("  (%d clusters)\n", nclusters);
                }
            }
        }
    }
}
