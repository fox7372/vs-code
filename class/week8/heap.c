#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
void exchange(int*a,int*b)
{
    int temp=*a;
    *a=*b;
    *b=temp;
}

void sort(int i,int*ptr,int n)
{
    int largest=i;
    if((2*i+1)<n&&ptr[largest]<ptr[2*i+1])largest=2*i+1;
    if((2*i+2)<n&&ptr[largest]<ptr[2*i+2])largest=2*i+2;
    if(largest!=i)
    {
        exchange(&ptr[i],&ptr[largest]);
        order(largest,ptr,n);
    }
}

void heap_sort(int n,int*ptr)
{
    assert(ptr!=NULL);
    for(int i=n/2-1;i>=0;i--)
    {
        order(i,ptr,n);
    }
    int index=n-1;
    for(int i=0;i<n-1;i++)
    {
        exchange(&ptr[index],&ptr[0]);
        order(0,ptr,index);
        index--;
    }
}


void main()
{
    printf("the number of data");
    int n=0;
    scanf("%d",&n);
    int*ptr=(int*)malloc(n*sizeof(int));
    assert(ptr!=NULL);
    for(int i=0;i<n;i++)
    {
        scanf("%d",&ptr[i]);
    }
    heap_sort(n,ptr);
    for(int i=0;i<n;i++)printf("%d\t",ptr[i]);
    free(ptr);   
}