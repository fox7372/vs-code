#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include"map.h"
#define MAX_QUEUE_SIZE 100

vertex* vertex_q[MAX_QUEUE_SIZE];
int front=0;
int rear=0;
int q_n=0;

void push_q(vertex*v)
{
	if(q_n==MAX_QUEUE_SIZE)
	{
		printf("Error: queue overflow\n");
		exit(1);
	}
	vertex_q[rear]=v;
	if(rear==MAX_QUEUE_SIZE-1)rear=0;
	else rear++;
	q_n++;
}

vertex* pop_q()
{
	if(q_n==0)
	{
		printf("Error: queue underflow\n");
		exit(1);
	}
	vertex* v=vertex_q[front];
	if(front==MAX_QUEUE_SIZE-1)front=0;
	else front++;
	q_n--;
	return v;
}

#endif

