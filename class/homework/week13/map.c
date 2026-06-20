/*
 * Graph algorithms — DFS / BFS / Prim / Kruskal
 * Supports both directed and undirected graph input, traversal, and MST
 */

#include"hash.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include"queue.h"
#include"stack.h"
#include"map.h"

#define N 10            /* max vertex count */
#define INF 0x3f3f3f3f  /* infinity, used for distance initialization in Prim */
#define NUB_EGDE 100    /* max edge count */

vertex* map[N]={0};      /* vertex array: map[i] stores pointer to the i-th vertex */
v* V_map[N]={0};         /* auxiliary array for Prim/Kruskal, tracks distance and predecessor per vertex */
edge* arr[NUB_EGDE];     /* edge array, used for sorting in Kruskal */

/* ==================== Heap operations (for Prim) ==================== */

/*
 * V_map_init: initialize the V_map array
 * Each vertex starts with distance INF and no predecessor
 */
void V_map_init(int n)
{
	for(int i=0;i<n;i++)
	{
        v*temp=(v*)malloc(sizeof(v));
		assert(temp!=NULL);
		temp->distance=INF;
		temp->ver=map[i];
		temp->pre=NULL;
		V_map[i]=temp;
	}
}

/*
 * exchange: swap two v* pointers (for heap adjustment)
 */
void exchange(v**a,v**b)
{
    v* temp=*a;
    *a=*b;
    *b=temp;
}

/*
 * make_heap: heapify the subtree rooted at i (min-heap)
 * Recursively bubble the smallest value up to the root
 */
void make_heap(v **ptr, int n, int i)
{
    int largest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;
    if (left < n && ptr[left]->distance < ptr[largest]->distance)
        largest = left;
    if (right < n && ptr[right]->distance < ptr[largest]->distance)
        largest = right;
    if (largest != i)
    {
        exchange(&ptr[i], &ptr[largest]);
        make_heap(ptr, n, largest);
    }
}

/*
 * heapify: build a min-heap from an array
 * Starts from the last non-leaf node and works upward
 */
void heapify(v **ptr, int n)
{
    for (int i = n/2 - 1; i >= 0; i--)
        make_heap(ptr, n, i);
}

/* ==================== 3-way quicksort (for Kruskal) ==================== */

/*
 * exchange_s: swap two edge* pointers
 */
void exchange_s(edge**a,edge**b)
{
    edge* temp=*a;
    *a=*b;
    *b=temp;
}

/*
 * fast_sort: 3-way quicksort, sorting edges by distance (weight)
 * Partitions the array into less-than, equal-to, and greater-than the pivot,
 * then recursively sorts the left and right partitions
 */
void fast_sort(edge**ptr,int left ,int right )
{
    if (left >= right) return;
    int length=right-left;
    int t=ptr[(rand()%length)+left]->distance;  /* random pivot */
    int r=left-1,l=left-1;
    int tail=right;
    for(int i=left;i<=tail;)
    {
         if(ptr[i]->distance<=t)
         {
            if(ptr[i]->distance==t)
            {
                exchange_s(&ptr[i],&ptr[r+1]);
                r++;
                i++;
            }
            else
            {
                exchange_s(&ptr[i],&ptr[++l]);
                r++;
                i++;
            }
         }
         else
         {
            exchange_s(&ptr[i],&ptr[tail--]);
         }
    }
    fast_sort(ptr,left,l);
    fast_sort(ptr,tail+1,right);
}

/* ==================== Vertex / Edge basic operations ==================== */

/*
 * create_vertex: create a new vertex
 * Allocates memory, copies the name, and initializes an edge hash table
 */
vertex* create_vertex(char v[])
{
	vertex* new_vertex=(vertex*)malloc(sizeof(vertex));
	assert(new_vertex!=NULL);
	strcpy(new_vertex->v,v);
	new_vertex->edges=create_hash_table();
	return new_vertex;
}

/*
 * search_map: find the vertex named v in the global vertex array map
 * Returns the index, or -1 if not found
 */
int search_map(char*v)
{
	for(int i=0;i<N;i++)
	{
		if(map[i]==NULL)continue;
		if (strcmp(map[i]->v, v) == 0) return i;
	}
	return -1;
}

int search_edge(char* key)
{
	char* temp=getname1(key);
	int index=search_map(temp);
	if(index==-1)return -1;
	int r=search_int(map[index]->edges,0,key);
	return r;
}

/*
 * getname1: extract the first vertex name from "{a,b}" or "(a,b)"
 * e.g. input "{a,b}" returns "a"
 */
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

/*
 * getname2: extract the second vertex name from "{a,b}" or "(a,b)"
 * Skips the first vertex and the comma, extracts until '}' or ')'
 */
char* getname2(char*key)
{
	char*name=(char*)malloc(sizeof(char)*15);
	assert(name!=NULL);
	char temp=*(key+1);
	int i=1;
	int id=0;
	while(temp!=',')
	{
		temp=*(key+1+i);
		i++;
	}
	/* skip the comma */
	temp=*(key+1+i);
	i++;
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

/*
 * store_vertex: enqueue or push all neighbors of vertex v
 * Used for DFS (st, stack) and BFS (qu, queue) traversal
 */
void store_vertex(vertex* v,enum structure_type type)
{
	if(v==NULL)return;
	for(int i=0;i<v->edges->size;i++)
	{
		if(v->edges->buckets[i]==NULL)continue;
		hash_node* temp=v->edges->buckets[i]->next;
		while(temp!=NULL)
		{
				char*index=getname2(temp->key);
				int idx=search_map(index);
				if(idx==-1)continue;
				if(type==qu)push_q(map[idx]);
				else if(type==st)push_s(map[idx]);
			temp=temp->next;
		}
	}
}

/* ==================== Prim's algorithm ==================== */

/*
 * prim: Prim's Minimum Spanning Tree algorithm
 * Starts from map[0]. At each step, picks the unvisited vertex with
 * the smallest distance and adds it to the MST. Uses a heap for efficiency.
 */
void prim(int n)
{
	vertex* temp=map[0];
	int n1=n;
	/* distance of the starting vertex is 0 */
	V_map[0]->distance=0;
	V_map[0]->pre=V_map[0]->ver;
	while(n1>0)
	{
		if(temp->edges==NULL||temp->edges->buckets==NULL)break;
        chain**c=temp->edges->buckets;
		/* scan all adjacent edges of the current vertex, update distances */
		for(int i=0;i<temp->edges->size;i++)
		{
			if(c[i]==NULL)continue;
			hash_node* index=c[i]->next;
			while(index!=NULL)
			{
                char*name=getname2(index->key);
				for(int j=0;j<n1;j++)
				{
					int v_idx=search_map(name);
					if(v_idx==-1)continue;
					if(V_map[j]->ver==map[v_idx])
					{
						if((V_map[j]->distance)>(index->value.i))
						{
							V_map[j]->pre=temp;
                            V_map[j]->distance=index->value.i;
						}
					}
				}
				index=index->next;
			}
		}
		/* move the vertex with smallest distance out of the heap */
		exchange(&V_map[0],&V_map[--n1]);
		heapify(V_map,n1);
        temp=V_map[0]->ver;
	}
}

/* ==================== Kruskal's algorithm ==================== */

/*
 * kruskal: Kruskal's Minimum Spanning Tree algorithm
 * Sorts all edges by weight (ascending), then greedily adds edges
 * that do not create a cycle.
 */
void kruskal(int n)
{
	/* c[] tracks vertices already in the set, used for cycle detection */
	char*c[n];
	memset(c,0,sizeof(c));
	int temp=0;
	V_map[0]->distance=0;
	V_map[0]->pre=V_map[0]->ver;
	/* sort all edges by weight */
    fast_sort(arr,0,n-1);
	for(int i=0;i<n-1;i++)
	{
        char* index=arr[i]->data;
		/* extract both endpoint names */
		char* name1=getname1(index);
		char* name2=getname2(index);
		int n1=1;  /* whether name1 is a new vertex */
		int n2=1;  /* whether name2 is a new vertex */
		/* check if each vertex is already in the set */
		for(int t=0;t<n;t++)
		{
			if(c[t] && strcmp(name1,c[t])==0)n1=0;
			if(c[t] && strcmp(name2,c[t])==0)n2=0;
		}
		/* add new vertices to the set */
		if(n1)
		{
			c[temp]=name1;
			temp++;
		}
		if(n2)
		{
			c[temp]=name2;
			temp++;
		}
		/* if at least one endpoint is new, this edge won't form a cycle */
		if(n1+n2!=0)
		{
			int j=0;
		    while(V_map[j]!=NULL)
		    {
			    if(V_map[j]->ver==map[search_map(name2)])
			    {
				    V_map[j]->distance=arr[i]->distance;
				    V_map[j]->pre=map[search_map(name1)];
				    break;
			    }
			    j++;
		    }
		}
	}

}

/*
 * printf_min_tree: print the Minimum Spanning Tree
 * Output format: "predecessor->vertex  weight"
 */
void printf_min_tree(int n)
{
	for(int i=0;i<n;i++)
	{
		printf("%s->%s  %d\n",V_map[i]->pre->v,V_map[i]->ver->v,V_map[i]->distance);
	}
}

/* ==================== DFS / BFS traversal ==================== */

/*
 * depthFirstSearch: Depth-First Search traversal
 * Starts from map[0], uses a stack to simulate recursion
 */
void depthFirstSearch()
{
	store_vertex(map[0],st);
	printf("%s\t",map[0]->v);
	while(s_n!=0)
	{
		vertex* v=pop_s();
		printf("%s\t",v->v);
		store_vertex(v,st);
	}
}

/*
 * breadthFirstSearch: Breadth-First Search traversal
 * Starts from map[0], uses a queue for level-order traversal
 */
void breadthFirstSearch()
{
	store_vertex(map[0],qu);
	printf("%s\t",map[0]->v);
	while(q_n!=0)
	{
		vertex* v=pop_q();
		printf("%s\t",v->v);
		store_vertex(v,qu);
	}
}

/* ==================== Input handling ==================== */

/*
 * input: read graph data from stdin
 * Each line: "{a,b} weight" or "(a,b) weight"
 * Creates vertices if they don't exist, stores edge info in the hash table
 */
void input(int num)
{
	int n=0;
	for(int i=0;i<num;i++)
	{
		int d;
		char* temp;
		char index[100]={};
		scanf("%s", index);
		scanf("%d", &d);
		/* extract both vertex names */
		temp=getname1(index);
		int index1=search_map(temp);
		char*temp2=getname2(index);
		int index2=search_map(temp2);
		/* create vertex if not found */
		if(index1==-1)
		{
			map[n]=create_vertex(temp);
			index1=n;
			n++;
		}
		if(index2==-1)
		{
			map[n]=create_vertex(temp2);
			n++;
		}
		/* store edge data */
		arr[i]=(edge*)malloc(sizeof(edge));
		assert(arr[i]!=NULL);
		strcpy(arr[i]->data,index);
		arr[i]->distance=d;
		insert_int(&map[index1]->edges,strdup(index),d);
	}
}

/* ==================== Main ==================== */

int main()
{
	printf("Enter edges in this format: directed graph (a,b); undirected graph {a,b}\n");
	int n=0;
	scanf("%d",&n);
	input(n);

	/* count actual vertices */
	int n1=0;
	while(map[n1]!=NULL) n1++;

	/* DFS */
	printf("DFS: ");
	depthFirstSearch();
	printf("\n");

	/* BFS */
	printf("BFS: ");
	breadthFirstSearch();

	printf("\n");

	/* Prim MST */
	V_map_init(n1);
	prim(n1);
	printf("Prim_MST:\n");
	printf_min_tree(n1);

    printf("\n");

	/* Kruskal MST */
    V_map_init(n1);
	kruskal(n);
	printf("Kruskal_MST:\n");
	printf_min_tree(n1);

	return 0;
}
