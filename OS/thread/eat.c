#include<thread.h>
#include<thread-sync.h>
#include<stdio.h>
#include<assert.h>
#include<stdint.h>

#define N 5

enum { THINKING, HUNGRY, EATING };
int state[N];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond[N];

static void test(int id) {
    int left = (id + N - 1) % N;
    int right = (id + 1) % N;
    if (state[id] == HUNGRY && state[left] != EATING && state[right] != EATING) {
        state[id] = EATING;
        pthread_cond_signal(&cond[id]);
    }
}

static void pickup(int id) {
    mutex_lock(&mutex);
    state[id] = HUNGRY;
    test(id);
    while (state[id] != EATING)
        pthread_cond_wait(&cond[id], &mutex);
    mutex_unlock(&mutex);
}

static void putdown(int id) {
    mutex_lock(&mutex);
    state[id] = THINKING;
    test((id + N - 1) % N);
    test((id + 1) % N);
    mutex_unlock(&mutex);
}

void* philosopher(void* arg) {
    int id = (int)(intptr_t)arg - 1;
    while (1) {
        printf("[Think] Philosopher %d is thinking.\n", id);
        pickup(id);
        printf("[Eat ] Philosopher %d is eating.\n", id);
        printf("[Done] Philosopher %d finished eating.\n", id);
        putdown(id);
    }
    return NULL;
}

int main() 
{
    for (int i = 0; i < N; i++)
        cond[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    for (int i = 0; i < N; i++)
        spawn(philosopher);
    join();
    return 0;
}
