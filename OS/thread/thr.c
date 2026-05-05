#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


struct ThreadData {
    int id;         
    int num1;       
    int num2;       
};


struct ThreadResult {
    int sum;
    long long product;
};

void* thread_task(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    printf("线程 %d 开始执行: 计算 %d 和 %d 的运算...\n", data->id, data->num1, data->num2);
    sleep(1); 
    struct ThreadResult* result = (struct ThreadResult*)malloc(sizeof(struct ThreadResult));
    
    if (result == NULL) {
        perror("malloc failed");
        pthread_exit(NULL);
    }
    result->sum = data->num1 + data->num2;
    result->product = (long long)data->num1 * data->num2;

    printf("线程 %d 执行完毕。\n", data->id);
    return (void*)result;
}

int main() {
    pthread_t tid1, tid2;
    void* thread_return; 
    struct ThreadData* data1 = (struct ThreadData*)malloc(sizeof(struct ThreadData));
    struct ThreadData* data2 = (struct ThreadData*)malloc(sizeof(struct ThreadData));
    
    data1->id = 1; data1->num1 = 10; data1->num2 = 5;
    data2->id = 2; data2->num1 = 8;  data2->num2 = 7;
    if (pthread_create(&tid1, NULL, thread_task, (void*)data1) != 0) {
        perror("Thread 1 create failed");
        return 1;
    }
    if (pthread_create(&tid2, NULL, thread_task, (void*)data2) != 0) {
        perror("Thread 2 create failed");
        return 1;
    }

    if (pthread_join(tid1, &thread_return) == 0) {
        struct ThreadResult* res = (struct ThreadResult*)thread_return;
        printf("主线程收到结果 1: Sum = %d, Product = %lld\n", res->sum, res->product);
        free(res); // 释放线程中 malloc 的内存
    }

    if (pthread_join(tid2, &thread_return) == 0) {
        struct ThreadResult* res = (struct ThreadResult*)thread_return;
        printf("主线程收到结果 2: Sum = %d, Product = %lld\n", res->sum, res->product);
        free(res); 
    }
    free(data1);
    free(data2);

    return 0;
}