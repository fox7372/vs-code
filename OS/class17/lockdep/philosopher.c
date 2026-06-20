#include <thread.h>
#include <thread-sync.h>
#include <stdio.h>
#include <unistd.h>

#define N 5

static mutex_t forks[N];

static void T_philosopher(int id) {
    int lhs = id - 1;
    int rhs = id % N;

    for (int round = 0; round < 3; round++) {
        mutex_lock(&forks[lhs]);
        mutex_lock(&forks[rhs]);

        printf("philosopher %d is eating\n", id);
        usleep(10000);

        mutex_unlock(&forks[rhs]);
        mutex_unlock(&forks[lhs]);
    }
}

int main(void) {
    for (int i = 0; i < N; i++)
        mutex_init(&forks[i]);

    for (int i = 0; i < N; i++)
        spawn(T_philosopher);
}
