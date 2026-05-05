#include<stdio.h>
#include<stdlib.h>
typedef struct node
{
    struct treenode*child;
    struct node* next;
}node;
typedef struct chain
{
    node*head;
    node*tail;
    int length;
}chain;
typedef struct treenode
{
    struct treenode*parent;
    int data;
    chain* c;
}treenode;
 
node*createnode(int data,treenode*parent)
{
    treenode* t=(treenode*)malloc(sizeof(treenode));
    t->data=data;
    t->parent=parent;
    chain*c=(chain*)malloc(sizeof(chain));
    c->head=NULL;
    c->length=0;
    c->tail=c->head;
    t->c=c;
    node*newnode=(node*)malloc(sizeof(node));
    newnode->next=NULL;
    newnode->child=t;
    return newnode;
}

void addnode(chain**c,node*newnode)
{
    if((*c)->tail==NULL)
    {
        (*c)->head=newnode;
        (*c)->tail=newnode;
    }
    else
    {
        (*c)->tail->next=newnode;
        (*c)->tail=newnode;
    }
    (*c)->length++;  
}
void printf_tree(treenode*top)
{
    if(top==NULL)return ;
    printf("%d\t",top->data);
    for(node*ptr=top->c->head;ptr!=NULL;ptr=ptr->next)
    {
        printf_tree(ptr->child);
    }
}
int size_tree(treenode*top)
{
    int index=1;
    if(top==NULL)return 0;
    for(node*ptr=top->c->head;ptr!=NULL;ptr=ptr->next)
    {
        index+=size_tree(ptr->child);
    }
    return index;
}

int high_tree(treenode*top)
{
    int high=1;
    if(top==NULL)return 0;
    for(node*ptr=top->c->head;ptr!=NULL;ptr=ptr->next)
    {
        high=high>(high_tree(ptr->child)+1)?high:(high_tree(ptr->child)+1);
    }
    return high;
}


void main()
{
    treenode* top=(treenode*)malloc(sizeof(treenode));
    top->data=1;
    top->parent=NULL;
    chain*C=(chain*)malloc(sizeof(chain));
    C->head=NULL;
    C->length=0;
    C->tail=C->head;
    top->c=C;
    node*h1=createnode(2,top);
    node*h2=createnode(3, top);
    addnode(&(top->c),h1);
    addnode(&(top->c),h2);
    node*m1=createnode(4,h1->child);
    addnode(&(h1->child->c),m1);
    printf_tree(top);
    printf("\n");
    printf("%d\t",size_tree(top));
    printf("%d\t",high_tree(top));
}