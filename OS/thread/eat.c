
#include "thread.h"

#define N 5

int cf[N] = {1,1,1,1,1};

pthread_mutex_t permit_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t permit_cond = PTHREAD_COND_INITIALIZER;

void* philosopher(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % N;

    while (1) 
    {

        printf("[Think] Philosopher %d is thinking.\n", id);
        pthread_mutex_lock(&permit_mutex);
        while (cf[left]==0||cf[right]==0) 
        {
            printf("[Wait ] Philosopher %d is waiting for a dining ...\n", id);
            pthread_cond_wait(&permit_cond, &permit_mutex);
        }
        cf[left]=0;
        cf[right]=0;
        printf("[Eat ] Philosopher %d is eating with chopsticks %d & %d.\n", id, left, right);
        printf("[Done] Philosopher %d finished eating.\n", id);
        cf[left]=1;
        cf[right]=1;
        pthread_cond_broadcast(&permit_cond);
        pthread_mutex_unlock(&permit_mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[N];
    int ids[N];
    for (int i = 0; i < N; i++)
    {
        {
        ids[i] = i;
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
        }
        pthread_join(threads[i], NULL);
    }
    return 0;
}
