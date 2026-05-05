#include"thread.h"

int store(atomic_int  *n,int m)
{
     if(*n==m)return 1;
     *n=m;
     return 0;
}
//自旋锁
atomic_int flag=0;
void lock()
{
    while(store(&flag,1));
}

void unlock()
{
   store(&flag,0);
}
//互斥锁
// 1. 定义互斥锁变量
pthread_mutex_t mutex;

void* thread_func(void* arg) {
    pthread_mutex_lock(&mutex); // 尝试加锁，如果被占用，线程在这里阻塞（休眠）
    
    // --- 临界区开始 ---
    // 操作共享资源（如全局变量、文件等）
    // --- 临界区结束 ---
    
    pthread_mutex_unlock(&mutex); // 解锁，如果有其他线程在等，唤醒它
    return NULL;
}
#define N 10000000
atomic_int  a=0;
void sum1(int i)
{
    lock();
    for(int j=0;j<N;j++)a++;
    unlock();
}
int main()
{
   pthread_mutex_init(&mutex, NULL);
   create(sum1);
   create(sum1);
   pthread_mutex_destroy(&mutex);
   join();
   printf("%d\n",a);
}
