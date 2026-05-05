#include"tree.h"

void exchange(int*a,int*b)
{
    int temp=*a;
    *a=*b;
    *b=temp;
}

void add_heap(tree_n**heap_head,int data)
{
    tree_n* ptr=creat_tnode(data);
    if(data>(*heap_head)->data)
    {
        exchange(&(ptr->data),&((*heap_head)->data));
        if((*heap_head)->left==NULL)
        {
            (*heap_head)->left=ptr;
            return ;   
        }
        if((*heap_head)->right==NULL)
        {
            (*heap_head)->right=ptr;
            return ;
        }
        add_heap(&((*heap_head)->left),ptr->data);
    }
    else
    {
        if((*heap_head)->left==NULL)
        {
              (*heap_head)->left=ptr;
            return ;   
        }
        if((*heap_head)->right==NULL)
        {
            (*heap_head)->right=ptr;
            return ;
        }
        add_heap(&((*heap_head)->right),ptr->data);
    }
}

void main()
{
    int arr[5]={3,2,4,6,1};
    tree_n*p=(tree_n*)malloc(sizeof(tree_n));
    p->left=NULL;
    p->right=NULL;
    p->data=arr[0];
    for(int i=1;i<5;i++)
    {
        add_heap(&p,arr[i]);
    }
    printf_tree(p);
}