/*
* Author: Qinbing Fu
* Implementation of binary search tree (BST) in C programming with three traversal orders:
* 1. in-order traversal: left->root->right
* 2. pre-order traversal: root->left->right
* 3. post-order traversal: left->right->root
*/

#include <stdio.h>
#include <stdlib.h>

#define SIZE 11

//BST node
struct node 
{
   //node data stored
   int data; 
	
   //children pointers
   struct node *leftChild;
   struct node *rightChild;
};

//declaration of processing
void insert(int);
struct node* search(int);
void pre_order_traversal(struct node*);
void inorder_traversal(struct node*);
void post_order_traversal(struct node*);


struct node *root = NULL;


//insert node 
//attention: iterative form of function, not recursive
void insert(int data)
{
   struct node *newNode = (struct node*) malloc(sizeof(struct node));
   struct node *current;
   struct node *parent;

   newNode->data = data;
   newNode->leftChild = NULL;
   newNode->rightChild = NULL;

   //if tree is empty
   if(root == NULL)
   {
      root = newNode;
   } 
   else
   {
      current = root;
      parent = NULL;

      while(1)
      {
         parent = current;
         
         //go to left of the tree
         if(data < parent->data)
         {
            current = current->leftChild;
            
            //insert to the left
            if(current == NULL)
            {
               parent->leftChild = newNode;
               return;
            }
         }
         else //go to right of the tree
         {
            current = current->rightChild;

            //insert to the right
            if(current == NULL)
            {
               parent->rightChild = newNode;
               return;
            }
         }
      }
   }
}

//search node (iterative)
struct node* search(int data)
{
   struct node *current = root;
   printf("Visiting elements: ");

   while(current->data != data)
   {
      if(current != NULL)
         printf("[ %d ] ",current->data);

      //go to left tree
      if(current->data > data)
      {
         current = current->leftChild;
      }
      //else go to right tree
      else
      {                
         current = current->rightChild;
      }

      //not found
      if(current == NULL)
      {
         return NULL;
      }
   }
   
   return current;
}

//preorder recursive
void pre_order_traversal(struct node* root)
{
   //root->left->right
   if(root != NULL)
   {
      printf("[ %d ] ",root->data);
      pre_order_traversal(root->leftChild);
      pre_order_traversal(root->rightChild);
   }
}

//inorder recursive
void inorder_traversal(struct node* root)
{
   //left->root->right
   if(root != NULL)
   {
      inorder_traversal(root->leftChild);
      printf("[ %d ] ",root->data);
      inorder_traversal(root->rightChild);
   }
}

//postorder recursive
void post_order_traversal(struct node* root)
{
   //left->right->root
   if(root != NULL)
   {
      post_order_traversal(root->leftChild);
      post_order_traversal(root->rightChild);
      printf("[ %d ] ", root->data);
   }
}

int main()
{
   int i;
   int array[SIZE] = { 10, 11, 20, 22, 30, 33, 40, 44, 50, 55, 18 };

   for(i = 0; i < SIZE; i++)
      insert(array[i]);

   printf("root [ %d ] \n", root->data);

   i = 11;
   struct node * temp = search(i);

   if(temp != NULL)
   {
      printf("[ %d ] Element found....", temp->data);
      printf("\n");
   }
   else
   {
      printf("Element (%d) not found ....\n", i);
   }

   i = 66;
   temp = search(i);

   if(temp != NULL)
   {
      printf("[ %d ] Element found....", temp->data);
      printf("\n");
   }
   else
   {
      printf("Element (%d) not found ....\n", i);
   }

   printf("\nPreorder traversal: ");
   pre_order_traversal(root);

   printf("\nInorder traversal: ");
   inorder_traversal(root);

   printf("\nPost order traversal: ");
   post_order_traversal(root);

   return 0;
}