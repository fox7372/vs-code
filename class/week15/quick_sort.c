/*
   Author: Qinbing Fu
   Description: Quick Sort example in C Programming
*/

#include <stdio.h>
#include <stdbool.h>

#define MAX 20

// unsorted integer array
int list[MAX] = {100,1,8,4,6,0,3,5,2,7,9,25,36,89,10,55,34,80,27,18};

// function prototype
void printline(int);
void display(void);
void swap(int, int);
int partition(int, int, int);
void quickSort(int, int);

void printline(int count)
{
   int i;
	
   for(i = 0;i < count-1;i++)
   {
      printf("=");
   }
	
   printf("=\n");
}

void display(void)
{
   int i;
   printf("[");
	
   // navigate through all items
   for(i = 0;i < MAX;i++)
   {
      printf("%d ",list[i]);
   }
	
   printf("]\n");
}

void swap(int num1, int num2)
{
   int temp = list[num1];
   list[num1] = list[num2];
   list[num2] = temp;
}

int partition(int left, int right, int pivot)
{
   int leftPointer = left - 1;
   int rightPointer = right;

   while(true)
   {
      while(list[++leftPointer] < pivot)
      {
         // do nothing
         // meditation...
      }
		
      while(rightPointer > 0 && list[--rightPointer] > pivot)
      {
         // do nothing
         // meditation...
      }

      if(leftPointer >= rightPointer)
      {
         break;
      }
      else
      {
         printf(" item swapped :%d,%d\n", list[leftPointer], list[rightPointer]);
         swap(leftPointer,rightPointer);
      }
   }
	
   printf(" pivot swapped :%d,%d\n", list[leftPointer], list[right]);
   swap(leftPointer,right);
   printf("Updated Array: ");
   display();
   return leftPointer;
}

void quickSort(int left, int right)
{
   if(right-left <= 0)
   {
      return;   
   }
   else
   {
      int pivot = list[right];
      int partitionPoint = partition(left, right, pivot);
      // recursive algorithm
      quickSort(left,partitionPoint-1);
      quickSort(partitionPoint+1,right);
   }
}

int main(void)
{
   printf("Input Array: ");
   display();
   printline(50);
   quickSort(0,MAX-1);
   printf("Output Array: ");
   display();
   printline(50);

   return 0;
}