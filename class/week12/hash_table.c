俄/*
* Author: Dr Qinbing Fu
* Hash table example in C programming
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE 20

//ADT type
struct DataItem
{
   int data;
   int key;
};

//ADT variable address
struct DataItem* hashArray[SIZE]; 
struct DataItem* dummyItem;
struct DataItem* item;

//functions prototype
int hashCode(int);
struct DataItem *search(int);
void insert(int,int);
struct DataItem *delete(struct DataItem*);
void display(void);

//hash function
//you can change this hash function
int hashCode(int key)
{
   return key % SIZE;
}

//search item in hash table
struct DataItem *search(int key)
{
   //get the hash code
   int hashIndex = hashCode(key);
	
   //move in array until an empty
   while(hashArray[hashIndex] != NULL)
   {
      if(hashArray[hashIndex]->key == key)
         return hashArray[hashIndex];
      //go to next cell
      ++hashIndex;
      //wrap around the table
      hashIndex %= SIZE;
   }
	
   return NULL;
}

//insect item into hash table
void insert(int key, int data)
{
   struct DataItem *item = (struct DataItem*) malloc(sizeof(struct DataItem));
   item->data = data; 
   item->key = key;

   //get the hash code
   int hashIndex = hashCode(key);

   //move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->key != -1)
   {
      //go to next cell
      ++hashIndex;
      //wrap around the table
      hashIndex %= SIZE;
   }

   hashArray[hashIndex] = item;
}

//delete item from hash table
struct DataItem* delete(struct DataItem* item)
{
   int key = item->key;

   //get the hash code
   int hashIndex = hashCode(key);

   //move in array until an empty
   while(hashArray[hashIndex] != NULL)
   {
      if(hashArray[hashIndex]->key == key)
      {
         struct DataItem* temp = hashArray[hashIndex];
         //assign a dummy item at deleted position
         hashArray[hashIndex] = dummyItem;
         return temp;
      }

      //go to next cell
      ++hashIndex;
      //wrap around the table
      hashIndex %= SIZE;
   }
	
   return NULL;
}

//display to console window
void display(void)
{
   int i;
   for(i=0; i<SIZE; i++)
   {
      if(hashArray[i] != NULL)
         printf(" [%d, %d]",hashArray[i]->key,hashArray[i]->data);
      else
         printf(" ^.^ ");
   }
	
   printf("\n");
}

int main()
{
   //write your own code here
   dummyItem = (struct DataItem*) malloc(sizeof(struct DataItem));
   dummyItem->data = -1;  
   dummyItem->key = -1; 

   insert(1, 11);
   insert(2, 22);
   insert(42, 4242);
   insert(4, 44);
   insert(12, 1212);
   insert(14, 1414);
   insert(17, 1717);
   insert(13, 1313);
   insert(37, 3737);

   display();
   item = search(37);

   if(item != NULL)
   {
      printf("Element found: %d\n", item->data);
   }
   else
   {
      printf("Element not found\n");
   }

   delete(item);
   item = search(37);

   if(item != NULL)
   {
      printf("Element found: %d\n", item->data);
   }
   else
   {
      printf("Element not found\n");
   }
}