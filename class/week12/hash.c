#include<stdio.h>
#include<stdlib.h>
#include<stdio.h>
#define HASH_SIZE 10 
typedef struct hash_node 
{
	int value;
	struct hash_node* next;
} hash_node;

typedef struct 
{
	int size;
	int  count;
	hash_node** buckets;
} hash_table;

hash_table* create_hash_table()
{
	hash_table* table = (hash_table*)malloc(sizeof(hash_table));
	table->size = HASH_SIZE;
	table->count = 0;
	table->buckets = (hash_node**)malloc(sizeof(hash_node*) * table->size);
	for (int i = 0; i < table->size; i++)
	{
		table->buckets[i] = NULL;
	}
	return table;
}

int hash_function(int value)
{
	return value % 10;
}

void insert(hash_table* table,int value)
{
	int index = hash_function(value);
	hash_node* new_node = (hash_node*)malloc(sizeof(hash_node));
	new_node->value = value;
	if(table->buckets[index] == NULL)
	{
		new_node->next = NULL;
		table->buckets[index] = new_node;
	}
	else
	{
		new_node->next = table->buckets[index];
		table->buckets[index] = new_node;
	}
	table->count++;
}

int main()
{
	hash_table* table = create_hash_table();
	insert(table, 5);
	insert(table, 15);
	insert(table, 25);
	for (int i = 0; i < 10; i++)
	{
		insert(table, i);
	}
	return 0;
}
