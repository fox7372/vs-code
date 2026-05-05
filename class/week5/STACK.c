#include<stdio.h>
#include<stdlib.h>
#define max 50

void push(int data,int*arr,int*tail)
{
    if ( *tail== max) 
    {
        printf("STACK is full, reallocating...\n");
        arr = (int*)realloc(arr, 2* sizeof(arr));
        if (arr == NULL) 
        {
            printf("Reallocation failed\n");
            exit(1);
        }
    }
    arr[*tail]=data;
    (*tail)++;
}
int pop(int*arr,int*tail)
{
    int data=0;
    if(*tail==0)
    {
        printf("STACK is emply");
    }
    else
    {
        data=arr[--(*tail)];
    }
    return data;
}
int main()
{
    int* arr = (int*)malloc(max * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    int tail = 0;
    push(1,arr,&tail);
    push(2,arr,&tail);
    push(3,arr,&tail);
    for(int i=2;i>=0;i--)
    {
        int  data=pop(arr,&tail);
        printf("%d\n",data);
    }
    free(arr);
    return 0;
} 
