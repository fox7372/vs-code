#ifndef STACK_H
#define STACK_H

#include<stdio.h>
#include<stdlib.h>
#include"map.h"
#define MAX_STACK_SIZE 100


vertex* vertex_s[MAX_STACK_SIZE];
int top=0;
int s_n=0;
void push_s(vertex*v)
{
	if(s_n==MAX_STACK_SIZE)
	{
		printf("Error: stack overflow\n");
		exit(1);
	}
	vertex_s[top++]=v;
	s_n++;
}

vertex* pop_s()
{
	if(s_n==0)
	{
		printf("Error: stack underflow\n");
		exit(1);
	}
	s_n--;
	return vertex_s[--top];
}

#endif

