#include <stdio.h>
#include <stdlib.h>

int main()
{
    int n;
    printf("num\n");
    scanf("%d",&n);
    int *p=(int*)malloc(n*sizeof(int));
    if(p==NULL)
    {
        printf("error");
        exit(1);
    }
    for(int i=0;i<n;i++)
    {
        scanf("%d",p+i);
    }
    for(int i=0;i<n;i++)
    {
        printf("%d\t",*(p+i));
    }
    free(p);
    p=NULL;
    return 0; 
}
