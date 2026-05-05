#include<stdio.h>
#include<stdlib.h>
typedef struct tree_n
{
	int data,high;
    struct tree_n* parent;
	struct tree_n* left;	
	struct tree_n* right;
}tree_n;
struct node;

typedef struct node
{
	tree_n* p;
	struct node* next;
}node;

node* creatnode(tree_n*p)
{
	node* newnode = (node*)malloc(sizeof(node));
	if (newnode == NULL) {
		printf("内存分配失败！\n");
		exit(1);
	}
	newnode->next = NULL;
	newnode->p = p;
	return newnode;
}


tree_n* creat_tnode(int data,int high,tree_n*parent)
{
	tree_n* node = (tree_n*)malloc(sizeof(tree_n));
	if (node == NULL) 
	{
		printf("二叉树节点内存分配失败！\n");
		exit(1);
	}
    node->high=high;
    node->parent=parent;
	node->data = data;
	node->left = NULL;
	node->right = NULL;
	return node;
}

void freetree(tree_n* p)
{
	if (p == NULL)return ;
	freetree(p->right);
	freetree(p->left);
	free(p);
}

int insert_tree(int data,tree_n*root)
{
    if(root->data>data)
    {
        if(root->left==NULL)
        {
            root->left=creat_tnode(data,root->high+1,root);
            return 0;
        }
        return insert_tree(data,root->left);
    }
    else if(root->data<data)
    {
        if(root->right==NULL)
        {
            root->right=creat_tnode(data,root->high+1,root);
            return 0;
        }
        return insert_tree(data,root->right);
    }

    return -1;
}

void printftree(tree_n* p)
{
	if (p != NULL)
	{
		printf("%d\t", p->data);
		printftree(p->left);
		printftree(p->right);
	}
}

int search_high(tree_n*root)
{
    if(root==NULL)return 0;
    int h1=search_high(root->left);
    int h2=search_high(root->right);
    return h1>=h2?h1+1:h2+1;
}

void search(tree_n* root,int high,node**head)
{
	if (root != NULL)
	{
	    if(root->high==high)
        {
            node* newnode=creatnode(root);
            if(*head==NULL)
            {
                *head=newnode;
            }
            else
            {
                node*p=*head;
                while(p->next!=NULL)
                {
                    p=p->next;
                }
                p->next=newnode;
            }
        }
		search(root->left,high,head);
		search(root->right,high,head);
	}
}
int func(tree_n*d)
{
    int index=(d->parent->left)==d?0:1;
    tree_n*temp=d->parent;
    if(index==0)
    {
        while(temp->parent&&(temp->parent->left)==temp)
        {
            temp=temp->parent;
        }
    }
    else if(index==1)
    {
        while(temp->parent&&(temp->parent->right)==temp)
        {
            temp=temp->parent;
        }
    }
    if(!temp->parent)return -1;
    return temp->parent->data;
}


void function(tree_n*root)
{
    int high=search_high(root);
    node*head=NULL;
    search(root,high,&head);
    node*temp=head;
    while(temp!=NULL)
    {
        int d1=func(temp->p);
        int d2=temp->p->data;
        if(d1>d2)printf("%d---%d\n",d2,d1);
        if(d2>d1)printf("%d---%d\n",d1,d2);
        temp=temp->next;
    }
}
int main()
{
    int arr[13]={31,45,14,52,42,6,21,73,47,26,37,33,8};
    tree_n*root=creat_tnode(arr[0],1,NULL);
    for(int i=1;i<13;i++)
    {
        insert_tree(arr[i],root);
    }
    printftree(root);
    printf("high=%d\n",search_high(root));
    function(root);
}