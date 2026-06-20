#include <thread.h>
#include <thread-sync.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NTASKS 6

struct edge {
    int src, dst;
} edges[] = {
    {0, 1},
    {0, 2},
    {0, 3},
    {1, 4},
    {2, 4},
    {4, 5},
};

sem_t vertex_sem[NTASKS];

void T_worker(int tid) {
    tid--;

    for (int i = 0; i < LENGTH(edges); i++) {
        if (edges[i].dst == tid) {
            P(&vertex_sem[tid]);
        }
    }

    // Some simulated computation
    printf("Computing node #%d...\n", tid);
    sleep(1);

    for (int i = 0; i < LENGTH(edges); i++) {
        if (edges[i].src == tid) {
            V(&vertex_sem[edges[i].dst]);
        }
    }
}

int main() {
    for (int i = 0; i < NTASKS; i++) {
        SEM_INIT(&vertex_sem[i], 0);
    }

    for (int i = 0; i < NTASKS; i++) {
        spawn(T_worker);
    }

    join();
    printf("Execution completed.\n");
}
