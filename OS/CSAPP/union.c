#include<stdio.h>
void main()
{
    union{
        double d;
        unsigned long l;
    }temp;
    temp.d=22.0;
    printf("%ld",temp.l);

}
