#include"Hash.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"queue.h"
#include"stack.h"
#define N 10
typedef struct
{
	char v[15];         // BUG: 原来是 char*，存 &temp(栈地址)，循环后失效。
	                // 改为 char，直接存字符值，不受栈生命周期影响。
	hash_table* edges;
}vertex;

vertex* map[N];
for(int i=0;i<N;i++)map[i]=NULL;



void depthFirstSearch();
void breadthFirstSearch();

char* getname1(char*key)
{
    char*name=(char*)malloc(sizeof(char)*15);
	char temp=*(key+1);
	int i=1;
	int id=0;
	while(temp!=',')
	{
		name[id++]=temp;
		temp=*(key+1+i);
		i++;
		if(id>=15)
		{
			printf("Error: name too long\n");
			exit(1);
		}
	}
	name[id]='\0';
	return name;
}
char* getname2(char*key)
{
	char*name=(char*)malloc(sizeof(char)*15);
	char temp=*(key+1);
	int i=1;
	int id=0;
	while(temp!=',')
	{
		temp=*(key+1+i);
		i++;
	}
	while(temp!='}')
	{
		name[id++]=temp;
		temp=*(key+1+i);
		i++;
		if(id>=15)
		{
			printf("Error: name too long\n");
			exit(1);
		}
	}
	name[id]='\0';
	return name;
}

vertex* create_vertex(char v[])
{
	vertex* new_vertex=(vertex*)malloc(sizeof(vertex));
	strcpy(new_vertex->v,v);
	new_vertex->edges=create_hash_table();
	return new_vertex;
}

int search(char*v,vertex* map[])
{
	for(int i=0;i<N;i++)
	{
		if(map[i]==NULL)continue;
		if (strcmp(map[i]->v, v) == 0) return i;
	}
	return -1;
}

int search_edge(char* key,vertex* map[])
{
	char* temp=getname1(key);
	int index=search(temp,map);
	if(index==-1)return -1;
	int r=search_int(map[index]->edges,0,key);
	return r;
}

enum structure_type{queue,stack};
void store_vertex(vertex* v,enum structure_type type)
{
	for(int i=0;i<N;i++)
	{
		if(v->edges->buckets[i]==NULL)continue;
		hash_node* temp=v->edges->buckets[i]->next;
		while(temp!=NULL)
		{
			char*index=getname2(temp->key);
			if(type==queue)push_q(map[search(index,map)]);
			else if(type==stack)push_s(map[search(index,map)]);
			temp=temp->next;
		}
	}
}
void depthFirstSearch()
{
	store_vertex(map[0],stack);
	while(s_n!=0)
	{
		vertex* v=pop_s();
		printf("%s\t",v->v);
		store_vertex(v,stack);
	}
}
void breadthFirstSearch()
{
	store_vertex(map[0],queue);
	while(q_n!=0)
	{
		vertex* v=pop_q();
		printf("%s\t",v->v);
		store_vertex(v,queue);
	}
}

int main()
{
	printf("Enter the number of edge: ");
	int num_edges,n=0;
	scanf("%d", &num_edges);
	for(int i=0;i<num_edges;i++)
	{
		int d;
		char* temp;
		char index[100]={};       // BUG: 原为 char* index(野指针)，scanf
		                          // 写入随机地址，破坏栈上其他变量。
		scanf("%s", index);
		scanf("%d", &d);
		temp=getname1(index);
		int index1=search(temp,map);
		if(index1==-1)
		{
			map[n]=create_vertex(temp);
			index1=n;
			n++;
		}
		insert_int(&map[index1]->edges,strdup(index),d);
		// BUG: 原为 &temp(栈上临时 char)，循环后栈地址被覆盖，
		// 改用 strdup(index) 堆拷贝，key 生命周期不受栈影响。
	}
	printf("%d\n",search_edge("{a,b}",map));
}
