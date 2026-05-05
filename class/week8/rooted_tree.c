/*
 * ADT Tree in C Programming
 * Authors: Renyuan Liu & Dr. Qinbing Fu
 * This code represents the linked-list based tree and its algorithms.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct tree_node
{
    //element
    char element;
    //number of children
    int node_count;
    //parent pointer
    struct tree_node* parent;
    //children
    struct linking_node* head; //point to first children
    struct linking_node* tail; //point to last children
}tree_node;


typedef struct linking_node
{
    struct linking_node* next;
    struct tree_node* node;
}linking_node;


tree_node *root;


//declaration of ADT tree operations
bool isLeaf(tree_node* target_node);
int childrenNums(tree_node* target_node);
bool hasBrother(tree_node* target_node);
void traverse(tree_node* target_node);
bool isEmpty(tree_node* target_node);
tree_node* attach(tree_node* target_node, char element);
void detach(tree_node* parent, char target_child_element);
int max(int a, int b);
int size(tree_node* target_node);
int height(tree_node* target_node);


//max
int max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}

//leaf node or not
bool isLeaf(tree_node* target_node)
{
    if(target_node->head) 
        return false;
    return true;
}

//return number of children
int childrenNums(tree_node* target_node)
{
    return target_node->node_count;
}

//if has brother nodes
bool hasBrother(tree_node* target_node)
{
    if(target_node->parent->node_count>1)
        return true;
    return false;
}

//traverse the tree nodes recursively
void traverse(tree_node* target_node)
{
    printf("%c  ",target_node->element);

    linking_node* temp = target_node->head;
    
    while(temp != NULL)
    {
        traverse(temp->node) ;
        temp = temp->next ;
    }
}

//return size of a rooted tree
int size(tree_node* target_node)
{
    if (target_node == NULL)
        return 0;

    int tree_size=1;
    
    linking_node* temp = NULL;
    for (temp=target_node->head;temp!=NULL;temp=temp->next)
    {
        tree_size += size(temp->node);
    }

    return tree_size;
}

//return height of a rooted tree
int height(tree_node* target_node)
{
    if (target_node == NULL)
        return -1;
    
    int tree_height=0;

    linking_node* temp=NULL;
    for(temp=target_node->head;temp!=NULL;temp=temp->next)
    {
        tree_height=max(tree_height, 1+height(temp->node));
    }

    return tree_height;
}

//is tree empty
bool isEmpty(tree_node* target_node)
{
    return (target_node == NULL);
}

//attach
tree_node* attach(tree_node* target_node, char element)
{
    tree_node* newNode = (tree_node*)malloc(sizeof(tree_node));
    linking_node* newLink = (linking_node*)malloc(sizeof(linking_node));

    newNode->element = element;
    newNode->parent = target_node;
    newNode->head = NULL;
    newNode->tail = NULL;
    newNode->node_count = 0;

    newLink->next = NULL;
    newLink->node = newNode;
    
    if (target_node->head == NULL)
        target_node->head = newLink;
    else
        target_node->tail->next = newLink;
    target_node->tail = newLink;
    target_node->node_count++;

    return newNode;
}

//detach
void detach(tree_node* parent, char target_child_element)
{
    linking_node* temp = parent->head;
    linking_node* prev = NULL;
    
    if(temp == NULL) //no children
    {
        printf("\nParent %c has no children...\n",parent->element);
        return;
    }
    else if (temp->next == NULL) //only one child
    {
        if (temp->node->element == target_child_element)
        {
            temp->node->parent = NULL;
            parent->head = NULL;
            parent->tail = NULL;
            parent->node_count = 0;
            printf("\n%c detached \n",temp->node->element);
            traverse(root);
        }
        else
        {
            printf("\nParent %c has no mathched child %c...\n",parent->element,target_child_element);
        }
        return;
    }
    else //has more than one children
    {
        while(temp->node->element != target_child_element)
        {
            if (temp->next == NULL)
            {
                printf("\nParent %c has no mathched child %c...\n",parent->element,target_child_element);
                return;
            }
            else
            {
                prev = temp;
                temp = temp->next;
            }
        }
    }

    //if target child found
    printf("\n%c detached \n",temp->node->element);
    temp->node->parent = NULL;
    if (temp == parent->head)
    {
        parent->head=parent->head->next;
    }
    else
    {
        prev->next=prev->next->next;
    }
    temp = parent->head;
    while (temp->next!=NULL)
    {
        temp=temp->next;
    }
    parent->tail=temp;
    parent->node_count--;

    traverse(root);
}

int main(void)
{
    printf("\nCurrent tree height: %d \n",height(root));

    //create root
    root = (tree_node*)malloc(sizeof(tree_node));
    root->element = 'A';
    root->parent = NULL;
    root->head = NULL;
    root->tail = NULL;
    root->node_count = 0;

    if (isEmpty(root))
        printf("Now tree is empty......");

    //attach tree node
    tree_node* tn_B = attach(root,'B');
    tree_node* tn_C = attach(root,'C');
    tree_node* tn_D = attach(tn_B,'D');
    tree_node* tn_E = attach(tn_B,'E');
    tree_node* tn_F = attach(tn_C,'F');
    tree_node* tn_G = attach(tn_C,'G');
    tree_node* tn_H = attach(tn_C,'H');
    tree_node* tn_I = attach(tn_C,'I');
    tree_node* tn_J = attach(tn_E,'J');
    tree_node* tn_K = attach(tn_E,'K');
    tree_node* tn_L = attach(tn_E,'L');
    tree_node* tn_M = attach(tn_K,'M');
    tree_node* tn_N = attach(tn_K,'N');

    traverse(root);
    printf("\nCurrent tree size: %d ",size(root));
    printf("\nCurrent tree height: %d \n",height(root));

    //detach tree node
    detach(root, 'C');
    printf("\nCurrent tree size: %d ",size(root));
    printf("\nCurrent tree height: %d \n",height(root));
    detach(tn_E, 'K');
    printf("\nCurrent tree size: %d ",size(root));
    printf("\nCurrent tree height: %d \n",height(root));
    detach(root, 'B');
    printf("\nCurrent tree size: %d ",size(root));
    printf("\nCurrent tree height: %d \n",height(root));

    return 0;
}