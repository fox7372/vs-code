#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include <assert.h>
#define max 100
typedef struct 
{
    int x;
    int y;
}arg;

void* add(void*t)
{
    arg*m=(arg*)t;
    
    int* g=(int*)malloc(sizeof(int));
    *g=m->x+m->y;
    pthread_exit((void*)g);
}

pthread_t arr[max];
int p=0;

int main()
{
    arg i={5,6};
    pthread_create(&arr[p],NULL,add,&i);
    void *m =NULL;
    pthread_join(arr[--p], &m);
    printf("%d\t", *(int*)m);
}
