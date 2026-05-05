#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"thread.h"

typedef struct data
{
    int coef;
    int power;
}data;
typedef struct node
{
    data D;
    struct node*next;
}node;
#define NUM 100
node*head[NUM]={0};

node* creatnode( data D)
{
    node*N=(node*)malloc(sizeof(node));
    assert(N!=NULL);
    N->D=D;
    N->next=NULL;
    return N;
}

void free_chain(node*head)
{
    if(head==NULL) return;
    node*c=head;
    node*n=NULL;
    while(c!=NULL)
    {
        n=c->next;
        free(c);
        c=n;
    }
    head=NULL;
}
void combine(int t)
{
    node*pc1=head[t];
    node*pc2=head[t+1];
    node*nhead=NULL;
    node*p=NULL;
    node*m=NULL;
    int i=0;
    while(pc1!=NULL&&pc2!=NULL)
    {
        if(pc1->D.power==pc2->D.power)
        {
            if(pc1->D.coef+pc2->D.coef!=0)
            {
                data d={pc1->D.coef+pc2->D.coef,pc1->D.power};
                m=creatnode(d);
                if(i==0)
                {
                    nhead=m;
                    p=nhead;
                    i++;
                }
                else
                {
                    p->next=m;
                    p=m; 
                }
                
            }
            pc1=pc1->next;
            pc2=pc2->next;
        }
        else
        {
            data ss=(pc1->D.power)<(pc2->D.power)?pc1->D:pc2->D;
            m=creatnode(ss);
            if(i==0)
            {
                nhead=m;
                p=nhead;
                i++;
            }
                else
            {
                p->next=m;
                p=m; 
            }
            if(pc1->D.power < pc2->D.power)
            pc1 = pc1->next;
            else
            pc2 = pc2->next;
        }
    }
    
    while(pc1!=NULL)
    {
        m=creatnode(pc1->D);
        p->next=m;
        p=m;
        pc1=pc1->next;
    }

    while(pc2!=NULL)
    {
        m=creatnode(pc2->D);
        p->next=m;
        p=m;
        pc2=pc2->next;
    }
    head[t/2]=nhead;
}
node*n2link(int n)
{
    if(n==1)return head[0];
    int j=0;
    int i=0;
    for(;i<n-1;i=i+2)
    {
        combine(i);
    }
    if(n%2) head[(i/2)+1]=head[n-1];
    node*p=n2link((n/2)+(n%2));
    return p;
}

void free_head(int n)
{
    for(int i=0;i<n;i++)
    {
        if(head[i]==NULL)continue;
        free_chain(head[i]);
    }
}


int main()
{
    int n=0;
    printf("number of CHAIN\n");
    scanf("%d",&n);
    assert(n>0&&n<=10);
    printf("not input number mean stop\n");
    for(int i=0;i<n;i++)
    {
        int ceof=0;
        int power=0;
        printf("ceof  power\n");
        scanf("%d %d",&ceof,&power);
        data d;
        d.coef=ceof;
        d.power=power;
        head[i]=creatnode(d);
        node*g=head[i];
        while(scanf("%d %d",&ceof,&power)==2)
        {   
            d.coef=ceof;
            d.power=power;
            node*s=creatnode(d);
            g->next=s;
            g=s;
        } 
         int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    //node*h = nlink(n);
    node*h = n2link(n);
    //node*h = link_nodes(head[0],head[1]);
    node*ptr=h;
    
    while(ptr->next!= NULL )
    {
        printf("%d*x^%d+",ptr->D.coef,ptr->D.power);
        ptr=ptr->next;
    }
    if(ptr != NULL)printf("%d*x^%d",ptr->D.coef,ptr->D.power);
    free_head(n);
}