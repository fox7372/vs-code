/*
 * Author: Dr Qinbing Fu
 * Min-Heap C Programming Using an Array structure
 */
 
#include <stdio.h>

typedef struct
{
    int *data;     // stored array
    int size;     // current size
    int capacity; // max capacity
} MinHeap;


// functions statement
void swap(int *a, int *b);  // Swap two variables
void moveDownRecursive(MinHeap *heap, int index);   // Swap down and recursive function
int pop(MinHeap *heap); // Pop from a min-heap
int top(MinHeap *heap); // Return top object of a min-heap
void moveUpRecursive(MinHeap *heap, int index); // Swap up and recursive function
void push(MinHeap *heap, int value);    // Push to a min-heap


void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void moveDownRecursive(MinHeap *heap, int index)
{
    int smallest = index;
    int left = 2 * index + 1;  // left child index
    int right = 2 * index + 2; // right child index

    // Find the smaller value from left and right children
    if (left < heap->size && heap->data[left] < heap->data[smallest])
    {
        smallest = left;
    }
    
    if (right < heap->size && heap->data[right] < heap->data[smallest])
    {
        smallest = right;
    }

    // Swap and Recursive if the minist value not found
    if (smallest != index)
    {
        swap(&heap->data[index], &heap->data[smallest]);
        moveDownRecursive(heap, smallest);
    }
}

int pop(MinHeap *heap)
{
    // heap underflow
    if (heap->size <= 0)
    {
        printf("Heap Underflow\n");
        return -1;
    }

    // Save root to return
    int root = heap->data[0];

    // Move the last element to top
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;

    // Recursive move down to restore a min-heap
    if (heap->size > 0)
    {
        moveDownRecursive(heap, 0);
    }

    return root;
}

int top(MinHeap *heap)
{
    // heap underflow
    if (heap->size <= 0)
    {
        printf("Heap Underflow\n");
        return -1;
    }
    
    // Save root to return
    int root = heap->data[0];
    
    return root;
}

void moveUpRecursive(MinHeap *heap, int index)
{
    // Stop recursive if it is root
    if (index == 0)
    {
        return;
    }

    // Calculate parent index
    int parent = (index - 1) / 2;

    // Swap and Recursive if its parent is bigger
    if (heap->data[index] < heap->data[parent])
    {
        swap(&heap->data[index], &heap->data[parent]);
        moveUpRecursive(heap, parent);
    }
}

void push(MinHeap *heap, int value)
{
    // Check if heap is full
    if (heap->size == heap->capacity)
    {
        printf("Heap Overflow\n");
        return;
    }

    // Push new element to heap
    heap->data[heap->size] = value;
    int currentIndex = heap->size;
    heap->size++;

    // Recursive move up to restore a min-heap
    moveUpRecursive(heap, currentIndex);
}

int main(void)
{
    // Create your own heap for practice
    // ^.^
    return 0;
}