/*
 * Author: Dr. Qinbing Fu
 * Date: 2026/03/08
 * Description: This exemplifies the use of struct data type.
*/

#include <stdio.h>
#include <stdlib.h>

struct point
{
    int x;
    int y;
};

struct abc
{
    int a;
    char b;
    char c;
};

// attention: self referential structures
struct self
{
    int p;
    struct self *ptr;
};
struct code
{
    int i;
    char c;
    struct code *ptr;
};

struct point edit(struct point p)
{
    (p.x)++;
    p.y = p.y + 5;
    return p;
}

void print(struct point p)
{
    printf("%d %d\n", p.x, p.y);
}

void print2(struct point *ptr)
{
    printf("%d %d\n", ptr->x, ptr->y);
}

void print3(struct abc arr[], int arr_size)
{
    int i;
    for(i=0; i<arr_size; i++)
    {
        printf("%d %c %c\n", arr[i].a, arr[i].b, arr[i].c);
    }
}

void edit2(struct point p)
{
    (p.x)++;
    (p.y)+=5;
}

struct point* func(int a, int b)
{
    struct point *ptr = (struct point*)malloc(sizeof(struct point));
    ptr->x = a;
    ptr->y = b + 10;
    return ptr;
}

int main()
{
    // 1
    struct point p1 = {23, 45};
    struct point p2 = {56, 90};
    //p1 = edit(p1);
    //p2 = edit(p2);
    edit2(p1);
    edit2(p2);
    print(p1);
    print(p2);
    printf("integer size: %d bytes\n", (int)sizeof(int));
    printf("struct point size %d bytes\n", (int)sizeof(struct point));

    // 2
    struct point *ptr1,*ptr2;
    ptr1=func(2,3);
    ptr2=func(6,9);
    print2(ptr1);
    print2(ptr2);
    free(ptr1);
    free(ptr2);
    ptr1=NULL;
    ptr2=NULL;

    // 3
    struct abc arr[2]={{1,'A','a'}, {2,'B','b'}};
    print3(arr, 2);

    // 4
    struct code var1;
    var1.i=66;
    var1.c='B';
    var1.ptr=NULL;
    struct code var2;
    var2.i=67;
    var2.c='C';
    var2.ptr=NULL;

    // magic
    var1.ptr=&var2;
    printf("%d %c\n", var1.ptr->i, var1.ptr->c);

    return 0;
}
