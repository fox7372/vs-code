#include<stdio.h>
#include<stdlib.h>
typedef struct node
{
    int data;
    struct node*next;
}node;
typedef struct queue
{
    node* head;
    node*tail;
    int length;
}queue;

node*createnode(int data)
{
	node* newnode = (node*)malloc(sizeof(node));
	if (newnode == NULL) 
    {
		printf("内存分配失败！\n");
		exit(1);
	}
	newnode->next=NULL;
	newnode->data = data;
	return newnode;
}
void enqueue(queue*q,node*d)
{
    if(q->head==NULL)
    {
        q->head=d;
        q->tail=q->head;
    }
    else
    {
         q->tail->next=d;
         q->tail=d;
    }
    q->length++;
}

int dequeue(queue*q)
{
    if(q->head==NULL)
    {
        printf("nothing in stack");
        return 0;
    }
    int d=q->head->data;
    node*f=q->head;
    q->head=q->head->next;
    free(f);
    q->length--;
    return d;
} 

void main()
{
    queue*h=(queue*)malloc(sizeof(queue));
    h->head=NULL;
    h->tail=NULL;
    h->length=0;
    node*m1=createnode(1);
    node*m2=createnode(2);
    node*m3=createnode(3);
    enqueue(h,m1);
    enqueue(h,m2);
    enqueue(h,m3);
    for(int i=0;i<3;i++)
    {
        printf("%d\t",dequeue(h));
    }

}
