#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>

void exchange(int*a,int*b)
{
    int temp=*a;
    *a=*b;
    *b=temp;
}

void heapify(int *ptr, int n, int i)
{
    int largest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;
    if (left < n && ptr[left] > ptr[largest])
        largest = left;
    if (right < n && ptr[right] > ptr[largest])
        largest = right;
    if (largest != i)
    {
        exchange(&ptr[i], &ptr[largest]);
        heapify(ptr, n, largest);
    }
}

void heap_sort(int *ptr, int l, int r)
{
    int n = r - l + 1;
    int *arr = ptr + l;

    for (int i = n/2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    for (int i = n-1; i > 0; i--)
    {
        exchange(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
}

int main()
{
    srand((unsigned)time(NULL));
    int n = 0;
    scanf("%d", &n);
    int *ptr = (int*)malloc(n * sizeof(int));
    assert(ptr != NULL);
    for (int i = 0; i < n; i++)
        scanf("%d", &ptr[i]);

    heap_sort(ptr, 0, n-1);

    for (int i = 0; i < n; i++)
        printf("%d\t", ptr[i]);
    free(ptr);
    return 0;
}
