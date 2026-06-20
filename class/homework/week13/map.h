#ifndef MAP_H
#define MAP_H

#include<stdio.h>

typedef struct
{
	char v[15];
	hash_table* edges;
}vertex;

typedef struct 
{
	vertex* ver;
	vertex*pre;
	int distance;
}v;

typedef struct 
{
    int distance;
	char data[100];
}edge;

//heap
void V_map_init(int n);
void exchange(v**a,v**b);
void make_heap(v **ptr, int n, int i);
void heapify(v **ptr, int n);
//fast_sort
void exchange_s(edge**a,edge**b);
void fast_sort(edge**ptr,int left ,int right );
//map
void depthFirstSearch();
void breadthFirstSearch();
vertex* create_vertex(char v[]);
int search_map(char*v);
int search_edge(char* key);
char* getname1(char*key);
char* getname2(char*key);
enum structure_type{qu,st};
void store_vertex(vertex* v,enum structure_type type);
void prim(int n);
void kruskal(int n );
void printf_min_tree(int n);
void input(int num);

#endif