#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
	int data;
	struct node*next;
}node;

node* creatnode(int data)
{
	node* newnode = (node*)malloc(sizeof(node));
	if (newnode == NULL) {
		printf("内存分配失败！\n");
		exit(1);
	}
	newnode->next=NULL;
	newnode->data = data;
	return newnode;
}
node* createchain(int arr[], int length)
{
	if (length <= 0) return NULL;
	node* head= creatnode(arr[0]);
	node* tail = head;
	for (int i = 1; i <length; i++)
	{
		node* newlie = creatnode(arr[i]);
		tail->next = newlie;
		tail= newlie;
	}
	return head;
}
void freechain(node* head)
{
	node* temp;
	while (head != NULL)
	{
		temp = head;
		head = head->next;
		free(temp);
	}
}

void enq(node**tail,int data)
{
    node p=creatnode(data);
    (*tail)->next=p;
    (*tail)=p;
}
node deq(node**head)
{
    if(*head=NULL)return NULL;
    node p=*head;
    *head=p->next;
    return p;
}