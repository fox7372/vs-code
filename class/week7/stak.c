#include<stdio.h>
#include<stdlib.h>
typedef struct node
{
    int data;
    struct node*next;
}node;
typedef struct stack
{
    node* top;
}stack;

node*createnode(char data)
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
void push(stack*p,node*d)
{
    if(p->top==NULL)p->top=d;
    else
    {
        d->next=p->top;
        p->top=d;
    }
}
char pop(stack*p)
{
    if(p->top==NULL)
    {
        printf("nothing in stack");
        return 0;
    }
    int m=p->top->data;
    node*n=p->top;
    p->top=p->top->next;
    free(n);
    return m;
}


void main()
{
    stack *t = (stack *)malloc(sizeof(stack));
    stack *s_int = (stack *)malloc(sizeof(stack));
    t->top = NULL;
    char arr[10] = { '(', '1', '+', '3', ')', '*', '(', '2', '+', '4' };
   for (int i = 0; i < 10; i++)
    {
        node *d = createnode(arr[i]);
        push(t,d);
    }
    int m=0;
    while(t->top!=NULL)
    {
        char data=pop(t);
        if(data=='(')m++;
        if(data==')')m--;
    }
    if(m!=0)printf("fasle");
}
