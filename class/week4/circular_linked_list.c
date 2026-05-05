/*
Author: Qinbing Fu
Date: 2026/03/23
Module: Data Structures and Algorithms
Workshop: Circular Linked Lists
This demo code shows the coding basics of circular single-linked list in C programming.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// linked list node
struct node 
{
   int data;
   int key;
	
   struct node *next;
};

// external pointers
struct node *head = NULL;
struct node *tail = NULL;
struct node *current = NULL;


// processes statement
bool isEmpty(void); // check if the list is empty
int length(void); // return the length of list (number of nodes)
void insertFirst(int key, int data); // insert at first
struct node *deleteFirst(void); // delete at first then return its address
void printList(void); // print the list in terminal


bool isEmpty(void)
{
   return head == NULL;
}

int length(void)
{
   int length = 0;

   //if list is empty
   if(head == NULL) 
   {
      return 0;
   }

   current = head->next;
   length++;

   // here different to single linked list
   while(current != head) 
   {
      length++;
      current = current->next;   
   }

   return length;
}

//insert node at the first location
void insertFirst(int key, int data)
{
   //create a new node
   struct node *newNode = (struct node*) malloc(sizeof(struct node));
   newNode->key = key;
   newNode->data = data;
	
   // attention here, difference
   if (isEmpty()) 
   {
      head = newNode;
      head->next = head;
      tail = head;
   } 
   else 
   {
      //point it to old first node
      newNode->next = head;
		
      //point first to new first node
      head = newNode;

      //update tail node
      tail->next = head;
   }  
   //printf("%p,%d,%d,%p\n",newNode,newNode->data,newNode->key,newNode->next);
   //printf("current head: %p\n",head);
   //printf("current tail->next: %p\n",tail->next);  
}

//delete first item
struct node *deleteFirst(void)
{

   //save reference to first node
   struct node *tempLink = head;
	
   if(head->next == head) 
   {  
      head = NULL;
      return tempLink;
   }     

   //mark next to first link as first 
   head = head->next;

   //update tail node
   tail->next = head;
	
   //return the deleted link
   return tempLink;
}

//display the list
void printList(void)
{

   struct node *ptr = head;
   printf("\n[ ");
	
   //start from the beginning
   if(head != NULL) 
   {
      // difference here
      while(ptr->next != head) 
      {
         //printf("(%d,%d) ",ptr->key,ptr->data);
         printf("(%p,%d,%d,%p) ",ptr,ptr->key,ptr->data,ptr->next);
         ptr = ptr->next;
      }

      //print the last node
      //printf("(%d,%d) ",ptr->key,ptr->data);
      printf("(%p,%d,%d,%p) ",ptr,ptr->key,ptr->data,ptr->next);
   }
	
   printf(" ]");
}

int main() 
{
   insertFirst(1,10);
   insertFirst(2,20);
   insertFirst(3,30);
   insertFirst(4,40);
   insertFirst(5,50);
   insertFirst(6,60); 

   printf("Original List: "); 
	
   //print list
   printList();

   //print list length
   printf("\nList length: %d\n", length());
   
   while(!isEmpty()) {            
      struct node *temp = deleteFirst();
      printf("\nDeleted value:");  
      printf("(%d,%d) ",temp->key,temp->data);
   }   
	
   printf("\nList after deleting all items: ");
   printList();
   
   return 0;
}