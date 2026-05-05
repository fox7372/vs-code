/*
Author: Dr. Qinbing Fu
Data Structures and Algorithms Workshop:
Circular Double-Linked Lists
This shows the coding basics of double-linked-list in C programming.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node 
{
   //data domain
   int data;
   int key;
	//address domain
   struct node *next;
   struct node *prev;
};

//this link always point to first node
struct node *head = NULL;

//this link always point to last node
struct node *last = NULL;

//this link always point to the current node
struct node *current = NULL;

//functions statement
bool isEmpty(void);
int length(void);
void displayForward(void);
void displayBackward(void);
void insertFirst(int key, int data);
void insertLast(int key, int data);
struct node* deleteFirst(void);
struct node* deleteLast(void);
struct node* delete(int key);
bool insertAfter(int key, int newKey, int data);

//is list empty
bool isEmpty(void) 
{
   return head == NULL;
}

int length(void)
{
   int length = 0;
   struct node *current;

	//counting from head to node before last
   for(current = head; current != last; current = current->next)
   {
      length++;
   }

   //counting the last
   length++;
	
   return length;
}

//display the list in from first to last
void displayForward(void)
{
   //start from the beginning
   struct node *ptr = head;
	
   //navigate till the end of the list
   printf("\n[ ");
	
   while(ptr != last) 
   {     
      //print data   
      //printf("(%d,%d) ",ptr->key,ptr->data);
      printf("(%p,%p,%d,%d,%p) ",ptr->prev,ptr,ptr->key,ptr->data,ptr->next);
      
      //traverse forward to next node
      ptr = ptr->next;
   }
	
   //print the last node
   //printf("(%d,%d) ",ptr->key,ptr->data);
   printf("(%p,%p,%d,%d,%p) ",ptr->prev,ptr,ptr->key,ptr->data,ptr->next);

   printf(" ]\n");
}

//display the list from last to first
void displayBackward(void)
{
   //start from the last
   struct node *ptr = last;
	
   //navigate till the start of the list
   printf("\n[ ");
	
   while(ptr != head) 
   {
      //print data
      //printf("(%d,%d) ",ptr->key,ptr->data);
      printf("(%p,%p,%d,%d,%p) ",ptr->next,ptr,ptr->key,ptr->data,ptr->prev);
		
      //traverse backward to previous node
      ptr = ptr ->prev;
   }
   
   //print the first node
   //printf("(%d,%d) ",ptr->key,ptr->data);
   printf("(%p,%p,%d,%d,%p) ",ptr->next,ptr,ptr->key,ptr->data,ptr->prev);

   printf(" ]\n");
}

//insert node at the first location
void insertFirst(int key, int data)
{
   //create a new node
   struct node *newNode = (struct node*) malloc(sizeof(struct node));
   newNode->key = key;
   newNode->data = data;
	
   if(isEmpty()) 
   {
      //make it the last node
      last = newNode;
   } 
   else 
   {
      //update first prev node
      head->prev = newNode;

      //point it to old first node
      newNode->next = head;
   }
	
   //point first node to new node
   head = newNode;

   //update links of first and last nodes
   head->prev = last;
   last->next = head;
}

//insert node at the last location
void insertLast(int key, int data)
{
   //create a node
   struct node *newNode = (struct node*) malloc(sizeof(struct node));
   newNode->key = key;
   newNode->data = data;
	
   if(isEmpty()) 
   {
      //make it the head node
      head = newNode;
   } 
   else 
   {
      //make node a new last node
      last->next = newNode;     
      
      //mark old last node as prev of new node
      newNode->prev = last;
   }

   //point last to new last node
   last = newNode;

   //update links of first and last nodes
   last->next = head;
   head->prev = last;
}

//delete first node
struct node* deleteFirst(void)
{
   //save reference to first node
   struct node *tempLink = head;
	
   //if only one node
   if(head->next == head)
   {
      head = NULL;
      last = NULL;
   } 
   else 
   {
      head->next->prev = last;
      head = head->next;
      last->next = head;
   }

   //return the deleted node
   return tempLink;
}

//delete node at the last location
struct node* deleteLast(void)
{
   //save reference to last node
   struct node *tempLink = last;
	
   //if only one node
   if(last->next == last) 
   {
      head = NULL;
      last = NULL;
   } 
   else 
   {
      last->prev->next = head;
      last = last->prev;
      head->prev = last;
   }
	
   //return the deleted node
   return tempLink;
}

//delete a node with given key
struct node* delete(int key)
{
   //start from the first node
   struct node* current = head;
   struct node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) 
   {
      //if it is last node	
      if(current->next == head) 
      {
         return NULL;
      } 
      else 
      {
         //store reference to current node
         previous = current;
			
         //move to next node
         current = current->next;             
      }
   }

   //found a match, update the node
   if(current == head)
   {
      //only one node in the list
      if(head->next == head)
      {
         head = NULL;
         last = NULL;
      }
      else
      {
         //change first to point to next node
         head = head->next;

         //update head and last
         last->next = head;
         head->prev = last;
      }
   }
   else if(current == last)
   {
      //change last to point to prev node
      last = current->prev;

      //update head and last
      head->prev = last;
      last->next = head;
   } 
   else 
   {
      //bypass the current node
      current->next->prev = current->prev;
      current->prev->next = current->next;
   }
	
   return current;
}

//insert a new node after a given key node
bool insertAfter(int key, int newKey, int data)
{
   //start from the first node
   struct node *current = head; 
	
   //if list is empty
   if(head == NULL) 
   {
      return false;
   }

   //navigate through list
   while(current->key != key) 
   {
      //if it is last node
      if(current->next == head) 
      {
         return false;
      } 
      else 
      {           
         //move to next node
         current = current->next;
      }
   }
	
   //create a node
   struct node *newNode = (struct node*) malloc(sizeof(struct node));
   newNode->key = newKey;
   newNode->data = data;

   //insert at the last position
   if(current == last) 
   {
      newNode->next = head;
      last->next = newNode;
      head->prev = newNode;
      newNode->prev = last;
      
      //update last
      last = newNode; 
   }
   //insert inside the list 
   else 
   {
      newNode->next = current->next;
      current->next = newNode;
      newNode->next->prev = newNode;
      newNode->prev = current;
   }

   //success
   return true; 
}

int main()
{
   //create list from the beginning
   insertFirst(6,60);
   insertFirst(5,50);
   insertFirst(4,40);
   insertFirst(3,30);
   insertFirst(2,20);
   insertFirst(1,10);

   //insert nodes at the end
   insertLast(7, 70);
   insertLast(8, 80);
   insertLast(9, 90);
   
   //length
   printf("\nList length is %d ......\n", length());

   //traverse forward
   printf("\nTraversing the Double Linked List (First to Last): ");  
   displayForward();
	
   //traverse backward
   printf("\n");
   printf("\nTraversing the Double Linked List (Last to first): "); 
   displayBackward();

   //deletion
   printf("\nList---after deleting first node: ");
   struct node *temp1 = deleteFirst();
   printf("\nDeleted node:");  
   printf("(%d,%d) ",temp1->key,temp1->data);
   displayForward();

   printf("\nList---after deleting last node: ");  
   struct node *temp2 = deleteLast();
   printf("\nDeleted node:");  
   printf("(%d,%d) ",temp2->key,temp2->data);
   displayForward();

   //insertion
   printf("\nList---insert after key(4): ");  
   insertAfter(4,10,100);
   displayForward();

   printf("\nList---after delete key(5): ");  
   delete(5);
   displayForward();
   
   printf("\nDouble linked lisk operation completed......");

   return 0;
}