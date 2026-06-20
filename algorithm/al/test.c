#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>

int main(int argc, char *argv[])
{
    int n = atoi(argv[1]); 
	srand((unsigned)time(NULL));
	printf("%d\t",n);
	for(int i=0;i<n;i++)
	{
		printf("%d\t",rand()%10000);
	}
}