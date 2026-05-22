#include"Hash.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#define N 10
typedef struct
{
	char v;         // BUG: 原来是 char*，存 &temp(栈地址)，循环后失效。
	                // 改为 char，直接存字符值，不受栈生命周期影响。
	hash_table* edges;
}vertex;


vertex* create_vertex(char v)
{
	vertex* new_vertex=(vertex*)malloc(sizeof(vertex));
	new_vertex->v=v;
	new_vertex->edges=create_hash_table();
	return new_vertex;
}

int search(char v,vertex* map[])
{
	for(int i=0;i<N;i++)
	{
		if(map[i]==NULL)continue;
		if (map[i]->v == v) return i;
	}
	return -1;
}

int search_edge(vertex*map[],char* key)
{
	char temp=*(key+1);
	int index=search(temp,map);
	if(index==-1)return -1;
	int r=search_int(map[index]->edges,0,key);
	return r;
}



int main()
{
	vertex* map[N];
	for(int i=0;i<N;i++)map[i]=NULL;
	printf("Enter the number of edge: ");
	int num_edges,n=0;
	scanf("%d", &num_edges);
	for(int i=0;i<num_edges;i++)
	{
		int d;
		char temp;
		char index[100]={};       // BUG: 原为 char* index(野指针)，scanf
		                          // 写入随机地址，破坏栈上其他变量。
		scanf("%s", index);
		scanf("%d", &d);
		temp=index[1];
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
	printf("%d\n",search_edge(map,"{a,b}"));
}
