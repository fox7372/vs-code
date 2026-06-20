#include<thread.h>
#include<thread-sync.h>
#include<stdio.h>

#define N 5

int cf[N] = {1,1,1,1,1};

pthread_mutex_t permit_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t permit_cond = PTHREAD_COND_INITIALIZER;

void* philosopher(void* arg) {
    int id = *(int*)arg;
    printf("[DEBUG] arg=%p, id=%d\n", arg, id);
    int left = id;
    int right = (id + 1) % N;
    while (1)
    {
        printf("[Think] Philosopher %d is thinking.\n", id);
        mutex_lock(&permit_mutex);
        while (cf[left]==0||cf[right]==0)
        {
            printf("[Wait ] Philosopher %d is waiting for a dining ...\n", id);
            pthread_cond_wait(&permit_cond, &permit_mutex);
        }
        cf[left]=0;
        cf[right]=0;
        mutex_unlock(&permit_mutex);
        printf("[Eat ] Philosopher %d is eating with chopsticks %d & %d.\n", id, left, right);
        printf("[Done] Philosopher %d finished eating.\n", id);
        mutex_lock(&permit_mutex);
        cf[left]=1;
        cf[right]=1;
        pthread_cond_broadcast(&permit_cond);
        mutex_unlock(&permit_mutex);
    }
    return NULL;
}

int main() {
    for (int i = 0; i < N; i++)
        spawn(philosopher);
    join();
    return 0;
}
