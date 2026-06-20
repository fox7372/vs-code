/*
 * trace-race.c — Data race test program for qemu-memtrace
 *
 * Build:
 *   gcc -O0 -g -o trace-race trace-race.c -lpthread
 *
 * Run under QEMU with memtrace plugin:
 *   qemu-x86_64 -plugin ./qemu-memtrace.so,elf=./trace-race \
 *                -- ./trace-race
 */
#include <stdio.h>
#include <thread.h>
#include <thread-sync.h>

#define N 100000

long sum = 0;

void T_sum() {
    for (int i = 0; i < N; i++) {
        sum++;
    }
}

int main() {
    spawn(T_sum);
    spawn(T_sum);

    join();

    printf("sum = %ld  (expected %d)\n", sum, 2 * N);
    return 0;
}
