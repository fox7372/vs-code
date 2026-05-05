#include"thread.h"
#define N 10000000
int a=0;
void sum(int i)
{
    for(int j=0;j<N;j++)a++;
}

int Sum(int arr[])
{
    int sum=0;
    int i=0;
    while(arr[i]!='\0')
    {
        sum+=arr[i++];
    
    }
    return sum;
}
int main()
{
    int arr[10]={1,2,3,4,5,6,7,8,9,10};
    data a={
        .iarr=arr[10],
        .
    };
    create_r(Sum,a);
    int* d=join_r();
    printf("%d\t",*d);
}
