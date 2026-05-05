
#include<time.h>
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
void printfchain(node* head)
{
	node* now = head;
	while (now != NULL)
	{
		printf("%d\t", now->data);
		now = now->next;

	}
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
node* addnode(node*head,int data)
{
	node* amm=head;
	while(amm->next!=NULL)
	{
		amm = amm->next;
	}
	node* mm = creatnode(data);
	mm->next = amm->next;
	amm->next = mm;
	return head;
}
node* delnode(node* head, int m)
{
	node* t = head;
	for (int i = 1; i < m; i++)
	{
		t = t->next;
	}
	node* k = head;
	for (int j = 1; j < m - 1; j++)
	{
		k = k->next;
	}
	k->next = t->next;
	free(t);
	return head;
}
node* exchangehead(node*head)
{
     node*temp=head;
     node*cru=temp->next;
     node*nhead=cru->next;
     temp->next=NULL;
     while(nhead->next!=NULL)
     {
         cru->next=temp;
         temp=cru;
         cru=nhead;
         nhead=nhead->next;
     }
     cru->next=temp;
     nhead->next=cru;
     return nhead;
}

typedef struct
{
    node* node;
    int i;
}t;
t Y(node* head)
{
    t resuit;
    resuit.node = NULL;
    node* m = head;
    node* n = head;
    n = n->next;
    m = n->next;
    int i = 0;
    while (m != NULL)
    {
        n = n->next;
        if(m->next!=NULL)
        {
           m=m->next->next;
        }
        else
        { 
           m=NULL ;
        }
        if (m == n)
        {
            i++;
            break;
        }
    }
    resuit.i = i;
    if (i)
    {
        m = head;
        while (m != n)
        {
            m = m->next;
            n = n->next;
        }
        resuit.node = m;
    }
    return resuit;
}



t judge(node* head1, node* head2)
{
    node* p1 = head1;
    t h1 = Y(head1);
    node* p2 = head2;
    t h2 = Y(head2);
    t resuit;
    resuit.i = 0;
    resuit.node = NULL;
    if (h1.i == 0 && h2.i == 0)
    {
        int n1 = 0, n2 = 0;
        while (p1 != NULL)
        {
            p1 = p1->next;
            n1++;
        }
        while (p2 != NULL)
        {
            p2 = p2->next;
            n2++;
        }
        node* np1 = (n1 - n2) > 0 ? head1 : head2;
        node* np2 = (np1 == head1) ? head2 : head1;
        int num = (unsigned int)(n1 - n2);
        for (int i = 0; i<num; i++)
        {
            np1 = np1->next;
        }
        while (np1 != NULL && np2 != NULL)
        {
            np1 = np1->next;
            np2 = np2->next;
            if (np1 == np2)
            {
                resuit.i = 1;
                resuit.node = np1;
                return resuit;
            }
        }
    }
    if (h1.i == 1 && h2.i == 1)
    {
        if (h1.node == h2.node)
        {
            resuit.i = 1;
            int n1 = 0, n2 = 0;
            while (p1 != h1.node)
            {
                p1 = p1->next;
                n1++;
            }
            while (p2 !=h1.node)
            {
                p2 = p2->next;
                n2++;
            }
            node* np1 = (n1 - n2) > 0 ? head1 : head2;
            node* np2 = (np1 == p1) ? head2 : head1;
            int num = (unsigned int)(n1 - n2);
            for (int i = 0; i<num; i++)
            {
                np1 = np1->next;
            }
            while (np1 != h1.node && np2 != h1.node)
            {
                np1 = np1->next;
                np2 = np2->next;
                if (np1 == np2)
                {
                    resuit.i = 1;
                    resuit.node = np1;
                    return resuit;
                }
            }
        }
        else
        {
            node* m = h2.node->next;
            while (m != h2.node)
            {
                m = m->next;
                if (m == h1.node)
                {
                    resuit.i = 1;
                    resuit.node = h1.node;
                    return resuit;
                }
            }

        }
    }
    if (h1.i == 1 && h2.i == 0)
    {
        resuit.i = 0;
        return resuit;
    }
}
int main()
{
    srand((unsigned int)time(NULL));
    int r = rand() % 10 + 1;
    int arr[10] = { 2,34,5,8,7,65,4,6,7,1 };
    node* head = createchain(arr, 10);
    /*node* p = head;
    node* g = head;
    for (int i = 0; i++; i < 10)
    {
        p = p->next;
    }
    for (int i = 0; i++; i < r)
    {
        g = g->next;
    }
    p->next = g;
    int ass[9] = { 0 };
    node* head2 = createchain(ass, 9);
    t m = judge(head, head2);
    if(m.i==1) printf("两条链相交");
    else printf("两条链不相交");
    */
    head=exchangehead(head);
    printfchain(head);
}
