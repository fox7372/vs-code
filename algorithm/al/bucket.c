#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
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

node*arr[10]={0};
void add_bucket(int i)
{
    int t=0;
	int index=i;
	node *g=creatnode(i);
    while(index/10!=0)
    {
        index=index/10;
        t++;
    }
    if(arr[t]==NULL)
	{
		arr[t]=g;
	}
	else
	{
        node*temp=arr[t];
		node*j=NULL;
		while(temp->next!=NULL)
		{
			if(i<=temp->data)
			{
				if(j==NULL)
				{
					arr[t]=g;
					g->next=temp;      
				}
				else
				{
                    j->next=g;
			        g->next=temp;
				}
				return ;
			}
            j=temp;
			temp=temp->next;	
		} 
		temp->next=g;
	}
}

void output()
{

	node*index=NULL;
	for(int i=0;i<=9;i++)
	{
		index=arr[i];
		while(index!=NULL)
		{
			printf("%d\t",index->data);
			index=index->next;
		}
	}
}   
void main()
{
    printf("the number of data");
    int n=0;
    scanf("%d",&n);
    int*ptr=(int*)malloc(n*sizeof(int));
    assert(ptr!=NULL);
    for(int i=0;i<n;i++)
    {
        scanf("%d",&ptr[i]);
    }
    for(int i=0;i<n;i++)
	{
		add_bucket(ptr[i]);
	}
	output();
	printf("\n");
    free(ptr);    
}