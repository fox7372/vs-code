#include <thread.h>
#include <thread-sync.h>
#include <stdio.h>
#include <stdlib.h>

#define N_LOCKS   32
#define N_THREADS 64
#define N_ROUNDS  100
#define MAX_PICK  8

static mutex_t locks[N_LOCKS];

static void pick_sorted(int buf[], int k) {
    int pool[N_LOCKS];
    for (int i = 0; i < N_LOCKS; i++) pool[i] = i;
    for (int i = 0; i < k; i++) {
        int j = i + rand() % (N_LOCKS - i);
        int tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
        buf[i] = pool[i];
    }
    for (int i = 1; i < k; i++) {
        int v = buf[i], j = i - 1;
        while (j >= 0 && buf[j] > v) { buf[j+1] = buf[j]; j--; }
        buf[j+1] = v;
    }
}

static void T_worker(int id) {
    (void)id;
    int buf[MAX_PICK];
    for (int r = 0; r < N_ROUNDS; r++) {
        int k = 2 + rand() % (MAX_PICK - 1);
        pick_sorted(buf, k);
        for (int i = 0; i < k; i++)
            mutex_lock(&locks[buf[i]]);
        for (int i = k - 1; i >= 0; i--)
            mutex_unlock(&locks[buf[i]]);
    }
}

int main(void) {
    for (int i = 0; i < N_LOCKS; i++)
        mutex_init(&locks[i]);

    for (int i = 0; i < N_THREADS; i++)
        spawn(T_worker);

    join();
    printf("pass test: %d threads x %d rounds.\n",
           N_THREADS, N_ROUNDS);
}
