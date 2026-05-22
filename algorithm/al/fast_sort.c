#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>

void exchange(int*a,int*b)
{
    int temp=*a;
    *a=*b;
    *b=temp;
}
void fast_order(int*ptr,int left ,int right )
{
    if (left >= right) return;
    int length=right-left;
    int t=ptr[(rand()%length)+left];
    int r=left-1,l=left-1;
    int tail=right;
    for(int i=left;i<=tail;)
    {
         if(ptr[i]<=t)
         {
            if(ptr[i]==t)
            {
                exchange(&ptr[i],&ptr[r+1]);
                r++;
                i++;
            }
            else
            {
                exchange(&ptr[i],&ptr[++l]);
                r++;
                i++;
            }
         }
         else
         {
            exchange(&ptr[i],&ptr[tail--]);
         }
    }
    fast_order(ptr,left,l);
    fast_order(ptr,tail+1,right);
}
 


void main()
{
    srand((unsigned)time(NULL));
    printf("the number of data");
    int n=0;
    scanf("%d",&n);
    int*ptr=(int*)malloc(n*sizeof(int));
    assert(ptr!=NULL);
    for(int i=0;i<n;i++)
    {
        scanf("%d",&ptr[i]);
    }
    fast_order(ptr,0,n-1);
    for(int i=0;i<n;i++)printf("%d\t",ptr[i]);
    free(ptr);
}