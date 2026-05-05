#include"thread.h"
pthread_mutex_t lk=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv=PTHREAD_COND_INITIALIZER;
int n=0;
int count=0;
void product()
{   
    while(1){
    pthread_mutex_lock(&lk);
    while(count==n) pthread_cond_wait(&cv,&lk);
    count++;
    printf("(");
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lk);}
}
void consume()
{  
    while(1){
    pthread_mutex_lock(&lk);
    while(count<=0) pthread_cond_wait(&cv,&lk);
    count--;
    printf(")");
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lk);}
}

int main()
{
   printf("please input n");
   scanf("%d",&n);
   create(product);
   create(consume);
   join();
}
