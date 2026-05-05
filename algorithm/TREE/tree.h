#include<stdio.h>
#include<stdlib.h>
typedef struct tree_n
{
	int data;
	struct tree_n* left;	
	struct tree_n* right;
}tree_n;
struct node;

typedef struct stack
{
	struct node* top;
}stack;

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
void push(stack* head, tree_n*data)
{
	if (head->top == NULL)
	{
		node* p = creatnode(data);
		head->top = p;
	}
	else
	{
		node* p = creatnode(data);
		p->next = head->top;
		head->top = p;
	}
}

tree_n* pop(stack* head)
{
	if (head->top == NULL)
	{
		return NULL;
	}
	node* f = head->top;
	tree_n* a = head->top->p;
	head->top = head->top->next;
	free(f);
	f = NULL;
	return a;
}

tree_n* creat_tnode(int data)
{
	tree_n* node = (tree_n*)malloc(sizeof(tree_n));
	if (node == NULL) 
	{
		printf("二叉树节点内存分配失败！\n");
		exit(1);
	}
	node->data = data;
	node->left = NULL;
	node->right = NULL;
	return node;
}
tree_n* creattree(int length,int arr[])
{
    if (length <= 0 || arr == NULL) {
        return NULL;
    }
    tree_n* head = creat_tnode(arr[0]);
    tree_n** ass = (tree_n**)malloc(length * sizeof(tree_n*));
    if (ass == NULL) {
        printf("内存分配失败！\n");
        exit(1);
    }
    ass[0] = head;
    for (int i = 1; i < length; i++)
    {
        ass[i] = creat_tnode(arr[i]);
    }
    int j = 0;
    for (int i = 1; i < length; )
    {
        ass[j]->left = ass[i++];
        if (i == length) break;
        ass[j++]->right = ass[i++];
    }
    free(ass);
    return head;
}
tree_n* creat_tree_n(int n, int arr[])
{
	int i = 0;
	stack s1;
	tree_n* head = creat_tnode(arr[i++]);
	tree_n* p = NULL;
	push(&s1, head);
	while (i <= n)
	{
		p = pop(&s1);
		tree_n* l = creat_tnode(arr[i++]);
		p->left = l;
		push(&s1, l);
		if (i == n)break;
		tree_n* r = creat_tnode(arr[i++]);
		p->right = r;
		push(&s1, r);
	}
	return head;
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

void freetree(tree_n* p)
{
	if (p == NULL)return ;
	freetree(p->right);
	freetree(p->left);
	free(p);
}

int jdg(tree_n* head,int p)
{
	if (head == NULL)
	{
		return 1;
	}
	int a=jdg(head->left,p);
	if (head->data <= p || a != 1)return 0;
	else p = head->data;
	return jdg(head->right,p);
}

int balance_tree2(tree_n* head)
{
	int j=1;
	if (head == NULL)return j;
	int min = -10000000;
	balance_tree2(head->left);
	if (head->data <= min)j = 0;
	else min = head->data;
	balance_tree2(head->right);
	return j;
}

tree_n* SameAn(tree_n* nd1, tree_n* nd2, tree_n* head)
{
	if (head == NULL || head == nd1 || head == nd2)
	{
		return head;
	}
	tree_n* left =SameAn(nd1, nd2, head->left);
	tree_n* right = SameAn(nd1, nd2, head->right);
	if (left != NULL || right != NULL)return head;
	return (left != NULL) ? left : right;
}
//1为凹 2为凸
void printfpress(int i, int n, int a)
{
	if (i > n)return;
	printfpress(i++, n, 1);
	printf("%d\t", a);
	printfpress(i++, n, 2);
}
//非递归遍历
void printf_tree(tree_n* head)
{
	stack sl;
	sl.top = NULL;
	push(&sl, head);
	while (sl.top != NULL)
	{
		tree_n* p = pop(&sl);
		printf("%d\t", p->data);
		
		if (p->right != NULL)push(&sl, p->right);
		if (p->left != NULL)push(&sl, p->left);
	}

}
typedef struct resuit
{
	int max;
	int j;
}resuit;
resuit balance_tree1(tree_n* head)
{
	if (head->left == NULL || head->right == NULL)
	{
		resuit r;
		r.j = 1;
		r.max = head->data;
		return r;
	}
	resuit b = balance_tree1(head->left);
	resuit a = balance_tree1(head->right);
	resuit final;
	final.j = 1;
	final.max = 0;
	if (a.max > head->data && b.max <= head->data && a.j + b.j == 2)
	{
		final.max = a.max;
	}
	else
	{
		final.j = 0;
	}
	return final;
}
