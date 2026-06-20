#include <thread.h>
#include <thread-sync.h>

#define N 5

mutex_t avail[N];

void T_philosopher(int id) {
    int lhs = (id + N - 1) % N;
    int rhs = id % N;

    while (1) {
        mutex_lock(&avail[lhs]);
        mutex_lock(&avail[rhs]);

        printf("Philosopher %d is eating\n", id);

        mutex_unlock(&avail[lhs]);
        mutex_unlock(&avail[rhs]);
    }
}

int main() {
    for (int i = 0; i < N; i++) {
        mutex_init(&avail[i]);
    }

    for (int i = 0; i < N; i++) {
        spawn(T_philosopher);
    }
}
