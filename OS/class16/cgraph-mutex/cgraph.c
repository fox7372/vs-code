#include <thread.h>
#include <thread-sync.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NTASKS 6

struct edge {
    int src, dst;
    mutex_t lk;
} edges[] = {
    {0, 1},
    {0, 2},
    {0, 3},
    {1, 4},
    {2, 4},
    {4, 5},
};

void T_worker(int tid) {
    tid--;

    // Acquire all locks of incoming edges
    for (int i = 0; i < LENGTH(edges); i++) {
        if (edges[i].dst == tid) {
            mutex_lock(&edges[i].lk);
        }
    }

    // Some simulated computation
    printf("Computing node #%d...\n", tid);
    sleep(1);

    // Releases all locks of outgoing edges
    for (int i = 0; i < LENGTH(edges); i++) {
        if (edges[i].src == tid) {
            mutex_unlock(&edges[i].lk);
        }
    }
}

int main() {
    for (int i = 0; i < LENGTH(edges); i++) {
        mutex_init(&edges[i].lk);
        mutex_lock(&edges[i].lk);
    }

    for (int i = 0; i < NTASKS; i++) {
        spawn(T_worker);
    }

    join();
    printf("Execution completed.\n");
}
