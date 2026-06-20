#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdint.h>


enum data_type{EMPTY,INT,FLOAT,CHAR};
typedef union 
{
	int i;
	float f;
	char* c;
}data;

typedef struct hash_node 
{
	enum data_type type;
	data value;
	char* key;
	struct hash_node* next;
} hash_node;

typedef struct 
{
	int size;
	hash_node* next;
} chain;
typedef struct 
{
	int size;
	int  count;
	chain** buckets;
} hash_table;

typedef struct 
{
	hash_node*top;
    int size;
}stack;

static inline stack* create_stack()
{
	stack* s=(stack*)malloc(sizeof(stack));
	assert(s!=NULL);
	s->top=NULL;
	s->size=0;
	return s;
}

hash_table* create_hash_table();
unsigned int hash_function(unsigned int x);
void hash_table_enlarge(hash_table** table);
void insert(hash_table** table1, hash_node* value);
uint64_t hash_float_to_bits(float f);
unsigned long hash_fnv(const char *str);
void insert_int(hash_table** table1,char* key,int value);
void insert_float(hash_table** table1,char* key,float value);
void insert_char(hash_table** table1,char* key,char* value);
int search_int(hash_table* table,int value,char* key);
float search_float(hash_table* table,float value,char* key);
char* search_char(hash_table* table,char* value,char* key);