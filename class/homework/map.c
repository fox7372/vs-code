#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<math.h>
#include <float.h>

//#include"queue.h"
//#include"stack.h"
//#include"map.h"

#define N 10            /* max vertex count */
#define INF 0x3f3f3f3f  /* infinity, used for distance initialization in Prim */
#define NUB_EGDE 100    /* max edge count */

typedef struct map
{
	int pre, ver;
	double distance;
}vertex;


//heap--------------------------------------------------
void exchange(vertex**a,vertex**b)
{
    vertex* temp=*a;
    *a=*b;
    *b=temp;
}

/*
 * make_heap: heapify the subtree rooted at i (min-heap)
 * Recursively bubble the smallest value up to the root
 */
void make_heap(vertex **ptr, int n, int i)
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
void heapify(vertex **ptr, int n)
{
    for (int i = n/2 - 1; i >= 0; i--)
        make_heap(ptr, n, i);
}


//--------------------------------------------------------------
vertex** map_init(int n)
{
    vertex** arr = (vertex**)malloc(n * sizeof(vertex*));
    for(int i = 0; i < n; i++)
    {
        arr[i] = (vertex*)malloc(sizeof(vertex));
        arr[i]->distance = DBL_MAX;
        arr[i]->ver = i;
    }
    return arr;
}

/* Fill the triangular distance matrix (non-overlapping rows).        */
/* Row i has (i+1) elements, storing distances from vertex i         */
/* to vertices 0..i.  For an undirected complete graph we use         */
/* ptr[max(a,b)][min(a,b)] to look up edge (a,b).                    */
void matrix_init(double*ptr[],int*x,int*y)
{
    for(int i=0;i<20;i++)
	{
		for(int j=0;j<=i;j++)
		{
            ptr[i][j]=sqrt((x[i]-x[j])*(x[i]-x[j])+(y[i]-y[j])*(y[i]-y[j]));
		}
	}
}

/* Dijkstra's shortest-path algorithm.                                */
/* num  = total vertex count                                         */
/* vert = start vertex (1-based)                                     */
/* ptr  = triangular distance matrix (non-overlapping rows)          */
vertex** dijkstra(int num,int vert,double*ptr[])
{
	vertex** map=map_init(num);
	int n=vert-1;

	/* initialise: set start-vertex distance to 0, all others to      */
	/* the direct edge from the start vertex                         */
	map[n]->distance=0;
	map[n]->pre=n;
	for(int i=0;i<num;i++)
	{
		if(i==n) continue;
		if(i<n)
			map[i]->distance=ptr[n][i];
		else
			map[i]->distance=ptr[i][n];
		map[i]->pre=n;
	}

	/* move start vertex out of the heap (it is already settled)     */
	exchange(&map[n],&map[--num]);
	heapify(map,num-1);
	int m=num;
	while(m>0)
	{
		heapify(map,m);
		int cur_vertex=map[0]->ver;
		double cur_dist=map[0]->distance;

		/* relax every edge (cur_vertex, i) for i still in the heap  */
		for(int i=0;i<=num;i++)
		{
			if(i==cur_vertex) continue;
			/* edge weight from the triangular matrix                */
			double d=(i<cur_vertex)?ptr[cur_vertex][i]:ptr[i][cur_vertex];
			for(int j=0;j<m;j++)
			{
				if(map[j]->ver==i&&(d+cur_dist)<map[j]->distance)
				{
					map[j]->distance=d+cur_dist;
					map[j]->pre=cur_vertex;
					break;
				}
			}
		}
		exchange(&map[0],&map[--m]);
		heapify(map,m-1);
	}
	return map;
}


void printf_map(int n,vertex**map)
{
	for(int i=0;i<n;i++)
	{
		printf("%d->%d  %f\n",map[i]->pre,map[i]->ver,map[i]->distance);
	}
}

int main()
{
	int x[20]={82,91,12,92,63,9,28,55,96,97,15,98,96,49,80,14,42,92,80,96};
    int y[20]={66,3,85,94,68,76,75,39,66,17,71,3,27,4,9,83,70,32,95,3};

	/* allocate the triangular matrix: 20 rows, row i has (i+1) elements */
	/* total = 1+2+...+20 = 210                                        */
	double*p=(double*)malloc(210*sizeof(double));
	assert(p!=NULL);
	double*ptr[20];
	int offset=0;
	for(int i=0;i<20;i++)
	{
		ptr[i]=p+offset;
		offset+=i+1;
	}

	matrix_init(ptr,x,y);
	vertex**map =dijkstra(20,2,ptr);
    printf_map(20,map);
}
