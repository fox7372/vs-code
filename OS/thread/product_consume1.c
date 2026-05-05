#include"thread.h"
#include<assert.h>
pthread_mutex_t lk=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv=PTHREAD_COND_INITIALIZER;
int n=1;
int c2=0;
int c1=0;

void product()
{   
    while(1){
    pthread_mutex_lock(&lk);
    while(c1>=n) pthread_cond_wait(&cv,&lk);
    c1++;
    assert(c1==1);
    printf("<");
    fflush(stdout);
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&lk);}
}
void consume1()
{
    while(1){
    pthread_mutex_lock(&lk);
    while(c2>=n||c1!=1) pthread_cond_wait(&cv,&lk);
    c2++;
    assert(c2==1);
    printf("-");
    fflush(stdout);
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&lk);}
}
void consume2()
{  
    while(1){
    pthread_mutex_lock(&lk);
    while(c2==0) pthread_cond_wait(&cv,&lk);
    c1--;
    c2--;
    assert(c1==0&&c2==0);
    printf(">");
    fflush(stdout);
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&lk);}
}
int main()
{
   create(product);
   create(consume1);
   create(consume2);
   join();
}
