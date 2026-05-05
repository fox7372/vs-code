#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define PATH_MAX 4096
#define LINE_MAX 1024

void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <pid>\n", prog);
    fprintf(stderr, "Dump the memory of a process to a binary file.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    pid_t pid = atoi(argv[1]);
    if (pid <= 0) {
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        return 1;
    }

    // Check if process exists
    if (kill(pid, 0) != 0) {
        fprintf(stderr, "Process %d does not exist: %s\n", pid, strerror(errno));
        return 1;
    }

    char maps_path[PATH_MAX];
    char mem_path[PATH_MAX];
    char output[PATH_MAX];

    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    snprintf(output, sizeof(output), "pid_%d.dump", pid);

    // Open memory file
    int mem_fd = open(mem_path, O_RDONLY);
    if (mem_fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", mem_path, strerror(errno));
        fprintf(stderr, "Try running with sudo or as root.\n");
        return 1;
    }

    // Open maps file
    FILE *maps = fopen(maps_path, "r");
    if (!maps) {
        fprintf(stderr, "Failed to open %s: %s\n", maps_path, strerror(errno));
        close(mem_fd);
        return 1;
    }

    // Open output file
    FILE *out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "Failed to create %s: %s\n", output, strerror(errno));
        fclose(maps);
        close(mem_fd);
        return 1;
    }

    char line[LINE_MAX];
    unsigned long total_dumped = 0;
    unsigned long region_count = 0;

    printf("Dumping memory of process %d...\n", pid);

    while (fgets(line, sizeof(line), maps)) {
        unsigned long start, end;
        char perms[8];
        char path[PATH_MAX] = {0};

        int n = sscanf(line, "%lx-%lx %7s %*x %*x:%*x %*d %4095s", &start, &end, perms, path);
        (void)n; // unused

        // Skip non-readable regions and guard pages
        if (perms[0] != 'r')
            continue;

        // Skip large shared mapped files (optional: reduce dump size)
        // Skip vsyscall/vvar/kernel pages
        if (strstr(path, "[vsyscall]") || strstr(path, "[vvar]") ||
            strstr(path, "[vdso]") || strstr(path, "[vvar]"))
            continue;

        size_t size = end - start;

        // Seek to the region start
        if (lseek(mem_fd, start, SEEK_SET) == (off_t)-1) {
            fprintf(stderr, "  Warning: cannot seek to 0x%lx, skipping\n", start);
            continue;
        }

        // Read memory content
        unsigned char *buf = malloc(size);
        if (!buf) {
            fprintf(stderr, "  Warning: malloc failed for %zu bytes, skipping 0x%lx-0x%lx\n",
                    size, start, end);
            continue;
        }

        ssize_t bytes_read = read(mem_fd, buf, size);
        if (bytes_read <= 0) {
            free(buf);
            continue;
        }

        // Write a small header per region for identification
        fwrite(&start, sizeof(start), 1, out);
        fwrite(&end, sizeof(end), 1, out);
        fwrite(buf, 1, bytes_read, out);

        total_dumped += bytes_read;
        region_count++;

        printf("  [%lu] 0x%016lx - 0x%016lx  %7s  %zu bytes  %s\n",
               region_count, start, end, perms, bytes_read,
               path[0] ? path : "");

        free(buf);
    }

    fclose(maps);
    close(mem_fd);
    fclose(out);

    printf("\nDone! Dumped %lu regions, %lu total bytes.\n", region_count, total_dumped);
    printf("Output file: %s\n", output);

    return 0;
}
