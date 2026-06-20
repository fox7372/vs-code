/*
   Author: Qinbing Fu
   Description: Merge Sort example in C Programming
*/

#include <stdio.h>
#include <time.h>

#define MAX 19

// unsorted integer array
// try your own array and increase the number of elements
int list[MAX+1] = {100,1,8,4,6,0,3,5,2,7,9,25,36,89,10,55,34,80,27,18};
// extra memory space for merging as sorted array
int extraList[MAX];

// function prototype
void merging(int, int, int);
void mergeSort(int, int);

// merge two arrays
void merging(int low, int mid, int high)
{
   int l1, l2, i;

   for(l1 = low, l2 = mid + 1, i = low; l1 <= mid && l2 <= high; i++)
   {
      if(list[l1] <= list[l2])
         extraList[i] = list[l1++];
      else
         extraList[i] = list[l2++];
   }
   
   // move elements (if existing) left in sub-array
   while(l1 <= mid)
      extraList[i++] = list[l1++];
   
   // move elements (if existing) left in sub-array
   while(l2 <= high)
      extraList[i++] = list[l2++];

   // copy elements back to the target array(unsorted array)
   for(i = low; i <= high; i++)
      list[i] = extraList[i];
}

// recursive merge sort
void mergeSort(int low, int high)
{
   int mid;
   
   if(low < high)
   {
      // find the middle point
      mid = (low + high) / 2;
      // recursive algorithm
      mergeSort(low, mid);
      mergeSort(mid+1, high);
      merging(low, mid, high);
   }
   else
      return;
}

void main(void)
{ 
   int i;
   // attention: timer
   clock_t start, end;
   double processTime;

   printf("\nList before merge sorting\n");
   
   for(i = 0; i <= MAX; i++)
      printf("%d ", list[i]);

   // sorting with timer
   start = clock();
   mergeSort(0, MAX);
   end = clock();
   processTime = (double)(end - start) / CLOCKS_PER_SEC;

   printf("\nMerge sort process time: %.10f seconds\n", processTime);
   // increase the unsorted array to see run time
   
   printf("List after merge sorting\n");

   for(i = 0; i <= MAX; i++)
      printf("%d ", list[i]);
}