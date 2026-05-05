#include<stdio.h>
long mult2(long a,long b)
{
    long s=a*b;
    return s;
}
void mulstore(long d,long a,long*c)
{
    long t=mult2(a,d);
    *c=t;
}
int main()
{
    long d;
    mulstore(2,3,&d);
    printf("2*3=%ld\n",d);
    return 0;
}