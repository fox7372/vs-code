#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>

#define HASH_SIZE 10    // initial bucket count
#define max 3           // max load factor before enlargement

// supported value types
enum data_type{EMPTY,INT,FLOAT,CHAR};

// type-erased value container (tagged by data_type)
typedef union
{
	int i;
	float f;
	char* c;
}data;

// hash node: key-value pair with linked list pointer
typedef struct hash_node
{
	enum data_type type;
	data value;
	char* key;          // NULL means hash by value itself
	struct hash_node* next;
} hash_node;

// bucket: chain head + size
typedef struct
{
	int size;           // nodes in this chain
	hash_node* next;
} chain;

// hash table with separate chaining
typedef struct
{
	int size;           // number of buckets
	int  count;         // total elements
	chain** buckets;
} hash_table;

// stack used during rehashing to collect live nodes
typedef struct
{
	hash_node*top;
    int size;
}stack;

// --- stack helpers (collect live nodes during rehash) ---

stack* create_stack()
{
	stack* s=(stack*)malloc(sizeof(stack));
	s->top=NULL;
	s->size=0;
	return s;
}

void push(stack**p,hash_node* d)
{
	d->next=(*p)->top;
	(*p)->top=d;
	(*p)->size++;
}

hash_node* pop(stack**p)
{
	if((*p)->top==NULL)
	{
		printf("nothing in stack");
		return NULL;
	}
	hash_node* n=(*p)->top;
	(*p)->top=(*p)->top->next;
	return n;
}


void free_chain(chain* node)
{
	if(node==NULL)return;
	hash_node* index = node->next;
	while(index != NULL)
	{
		hash_node* temp = index;
		index = index->next;
		free(temp);
	}
	free(node);
}

// --- forward declarations ---
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





hash_table* create_hash_table()
{
	hash_table* table = (hash_table*)malloc(sizeof(hash_table));
	table->size = HASH_SIZE;
	table->count = 0;
	table->buckets = (chain**)malloc(sizeof(chain*) * table->size);
	for (int i = 0; i < table->size; i++)
	{
		table->buckets[i] = NULL;
	}
	return table;
}

// --- hash functions ---

// Robert Jenkins' 32-bit integer hash (good avalanche)
unsigned int hash_function(unsigned int x)
{
      x = ~x + (x << 15);
      x ^= x >> 12;
      x += x << 2;
      x ^= x >> 4;
      x *= 2057;
      x ^= x >> 16;
      return x;
}

// reinterpret float bits as uint32 for hashing
uint64_t hash_float_to_bits(float f)
{
    union { float f; uint32_t bits; } u;
    u.f = f;
    return u.bits;
}

// FNV-1a string hash
unsigned long hash_fnv(const char *str)
{
  unsigned long h = 2166136261ul;
  while (*str)
  {
    h ^= (unsigned char)*str++;
    h *= 16777619ul;
  }
  return h;
}


// --- resize: double bucket count and rehash all nodes ---
void hash_table_enlarge(hash_table** table)
{
	hash_table* index=*table;
	int old_size=index->size;
	index->size*=2;
	index->buckets=(chain**)realloc(index->buckets,sizeof(chain*) * index->size);
	// realloc preserves old data; zero-init the new portion
	for(int i=old_size;i<index->size;i++)index->buckets[i]=NULL;
	// push all live nodes onto a stack
	stack* s=create_stack();
	for(int i=0;i<(index->size/2);i++)
	{
		if(index->buckets[i]==NULL)continue;
		hash_node* temp= index->buckets[i]->next;
		while(temp!=NULL)
		{
			hash_node*temp2=temp->next;
			push(&s,temp);
			temp=temp2;
		}
	}
	// clear old buckets
	for(int i=0;i<(index->size/2);i++)
	{
		free(index->buckets[i]);
		index->buckets[i]=NULL;
	}
	index->count=0;
	// rehash every node (insert uses the new size for bucket index)
	while(s->top!=NULL)
	{
		hash_node* temp=pop(&s);
		if(temp->type==INT)
		{
			insert(&index,temp);
		}
		else if(temp->type==FLOAT)
		{
			insert(&index,temp);
		}
		else if(temp->type==CHAR)
		{
			insert(&index,temp);
		}
	}

}

void free_hash_table(hash_table* table)
{
	for(int i=0;i<table->size;i++)
	{
		if(table->buckets[i]!=NULL)free_chain(table->buckets[i]);
	}
	free(table);
}

// --- typed insert wrappers ---

void insert_int(hash_table** table1,char* key,int value)
{
	hash_node* node=(hash_node*)malloc(sizeof(hash_node));
	node->type=INT;
	node->key=key;
	node->value.i=value;
	insert(table1,node);
}

void insert_float(hash_table** table1,char* key,float value)
{
	hash_node* node=(hash_node*)malloc(sizeof(hash_node));
	node->type=FLOAT;
	node->key=key;
	node->value.f=value;
	insert(table1,node);
}

void insert_char(hash_table** table1,char* key,char* value)
{
	hash_node* node=(hash_node*)malloc(sizeof(hash_node));
	node->type=CHAR;
	node->key=key;
	node->value.c=value;
	insert(table1,node);
}

// --- generic insert ---
// if key is set, hash by key string; otherwise hash by the value itself
void insert(hash_table** table1,hash_node* value)
{
	hash_table* table = *table1;
	if(table->count/table->size>=max)hash_table_enlarge(table1);
	int index;
	if(value->type==INT)
    {
		if(value->key==NULL)
		{
			index=hash_function(value->value.i)%table->size;
		}
		else
		{
			unsigned int i=hash_fnv(value->key);
      		index=hash_function(i)%table->size;
		}
	}
	else if(value->type==FLOAT)
	{
		if(value->key==NULL)
		{
			int temp=hash_float_to_bits(value->value.f);
			index=hash_function(temp)%table->size;
		}
		else
		{
			unsigned int i=hash_fnv(value->key);
			index=hash_function(i)%table->size;
		}
	}
	else if(value->type==CHAR)
	{
		if(value->key==NULL)
		{
			index=hash_function((int)(uintptr_t)value->value.c)%table->size;
		}
		else
		{
			unsigned int i=hash_fnv(value->key);
			index=hash_function(i)%table->size;
		}
	}
	// head-insert into the bucket chain
	if(table->buckets[index] == NULL)
	{
		chain* c = (chain*)malloc(sizeof(chain));
		c->size = 1;
		c->next = value;
		value->next = NULL;
		table->buckets[index] = c;
	}
	else
	{
		chain* c = table->buckets[index];
	    value->next = c->next;
	    c->next = value;
	    c->size++;
	}
	table->count++;
}


// --- search ---
// if key is set, look up by key; otherwise by value

int search_int(hash_table* table,int value,char* key)
{
	if(key==NULL)
	{
		int index=hash_function(value)%table->size;
		hash_node* temp=table->buckets[index]->next;
		while(temp!=NULL)
		{
			if(temp->type==INT&&temp->value.i==value)return 1;
			temp=temp->next;
		}
	}
	else
	{
		int index=hash_function(hash_fnv(key))%table->size;
		hash_node* temp=table->buckets[index]->next;
		while(temp!=NULL)
		{
			if(temp->type==INT&&temp->key==key)return temp->value.i;
			temp=temp->next;
		}
	}
	return -1;
}

float search_float(hash_table* table,float value,char* key)
{
	if(key==NULL)
	{
		int index=hash_function(value)%table->size;
		hash_node* temp=table->buckets[index]->next;
		while(temp!=NULL)
		{
			if(temp->type==FLOAT&&temp->key==key)return temp->value.f;
			temp=temp->next;
		}
	}
	else
	{
		int index=hash_function(hash_fnv(key))%table->size;
		hash_node* temp=table->buckets[index]->next;
		while(temp!=NULL)
		{
			if(temp->type==FLOAT&&temp->key==key)return temp->value.f;
			temp=temp->next;
		}
	}
	return -1;
}

char* search_char(hash_table* table,char* value,char* key)
{
	if(key==NULL)
	{
		int index=hash_function((int)(uintptr_t)value)%table->size;
		hash_node* temp=table->buckets[index]->next;
		while(temp!=NULL)
		{
			if(temp->type==CHAR&&temp->value.c==value)return temp->value.c;
			temp=temp->next;
		}
	}
	else
	{
		int index=hash_function(hash_fnv(key))%table->size;
		hash_node* temp=table->buckets[index]->next;
		while(temp!=NULL)
		{
			if(temp->type==CHAR&&temp->key==key)return temp->value.c;
			temp=temp->next;
		}
	}
	return NULL;
}

