#ifndef BTREES_H
#define BTREES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbool.h>
//#include <sys/timeb.h>
//#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#define MAX_KEY_LEN 15
#define BLOCK_TYPE 5
#define VERTEX_NUM 5000
#define ARC_NUM 2500




static int btree_size = 4;  // number of pointers of each btree_node
FILE *fp;
int index = 1;

//传递索引表数据结构
typedef struct
{
	int x;
	int y;
	struct key_type *Forpointer;
	//struct key_type *Repointer;
}IndexTable;
typedef IndexTable AdjList[ARC_NUM + 1];
AdjList array_list;

typedef struct
{
	int v;
	struct key_type *v_Forpointer;
}Index_array;
typedef Index_array ar_Index[VERTEX_NUM + 1];
ar_Index arindex;
int isexit_array_index[VERTEX_NUM + 1] = { 0 };

typedef struct L_Keypointer_Node
{  //用于Keypointer和KeyYpointer 分别把叶子块定位到原表的第某行
	int array_row;
	struct L_Keypointer_Node *next;
}L_Keypointer_Node, *L_Keypointer_List;
/*typedef struct L_Tuplepointer_Node
{
struct IndexTable *Tuplepointer;
struct L_Tuplepointer_Node *next;
}; //指向以当前块开头的索引表项（用于正向）*/

typedef struct key_type{
	int ivalue;
	L_Keypointer_List Keypointer;  //指向和当前块x相同的原表某些行 的链表的头结点（不为空）
	L_Keypointer_Node *Keypointer_last;
	//L_Keypointer_List L_KeyYpointer;  //指向和当前块y相同的原表某些行 的链表的头结点（不为空）
	//L_Tuplepointer_Node   *Tuplepointer_last; 
}key_type;
/*
* Btree btree_node
*/
typedef struct btree_node {//Btree btree_node
	void **pointers;
	key_type **keys;
	struct btree_node *parent;

	int num_keys;
	bool is_leaf;
	struct btree_node *next_leaf_node;
} btree_node;
/*
* store the result of the range search
*/
typedef struct bvalue{
	void *value;
	struct bvalue *next;
}bvalue_t, blist_t;
//typedef bvalue_t blist_t;
/*
* the rowid
*/
typedef struct btree_record {
	int value;
} btree_record;


typedef struct queue {//the queue
	int capacity;
	int front;
	int rear;
	int size;
	btree_node **items;
} queue;

typedef struct head_add_node{
	void *next;
	short int block_size;
}head_add_node;
typedef struct block_node{
	void *next;
}block_node;
typedef struct index_head {// the index head
	btree_node *root;     //the root of the index tree
	void *base;           //the base address of the mmap
	int keys_type;        //store the type of the key
	int node_count;       //the number of the keys
	long int offset;           //the offset of the index file
	head_add_node block[BLOCK_TYPE]; //point to the next free block
} index_head;
/*
* the data for test
*/

typedef struct data_btree_node{
	int no;
	int age;
	char name[12];
	char tel_number[12];
}data_btree_node;




/**********************************************************/
//the interface for using
index_head *create_index(char *database_name, char *table_name, char *key_name, int key_type, int count, char *file_name);
void *load_index(char *file_name);
btree_node *btree_insert(btree_node *root, key_type *key, int value, index_head *infor);//insert 
int btree_update(btree_node *root, key_type *key, int value, index_head *infor);//update
blist_t *btree_search_range(btree_node *root, key_type *low, key_type *high, index_head *infor);//range search  
btree_node *find_leaf(btree_node *root, key_type *key, index_head *infor);
btree_record *make_new_btree_record(int value, index_head *infor, key_type *key);
btree_node *make_new_btree_node(index_head *infor, key_type *key);
btree_node *make_new_leaf(index_head *infor, key_type *key);
btree_node *make_new_tree(key_type *key, int value, index_head *infor);
btree_node *make_new_root(btree_node *left, btree_node *right, key_type *key, index_head *infor, key_type *tkey);
btree_node *insert_into_parent(btree_node *root, btree_node *left, btree_node *right, key_type *key, index_head *infor);
void insert_into_btree_node(btree_node *nd, btree_node *right, int index, key_type *key, index_head *infor);
btree_node *insert_into_btree_node_after_splitting(btree_node *root, btree_node *nd, btree_node *right, int index, key_type *key, index_head *infor);
btree_node *insert_into_leaf_after_splitting(btree_node *root, btree_node *leaf, int index, key_type *key, btree_record *rec, index_head *infor);
void insert_into_leaf(btree_node *leaf, int index, key_type *key, btree_record *rec, index_head *infor);
int get_btree_node_index(btree_node *nd, void *base);
void distribute_btree_nodes(btree_node *nd, btree_node *neighbor, int nd_index, index_head *inforype, key_type *key);
int get_key_len(key_type *key, int type);
void *alloc_add(int length, int type, index_head *infor, key_type *key); //allocate the free address 
void initialize_keys(key_type *keys, key_type *key_input, int keys_type, key_type *tkey);
int str_cmp(key_type *a, key_type *b, int keys_type);
void init_head_leaf(index_head *infor);
void transitive_closure(btree_node *root, index_head *infor, double btime);
void Forward_transitive(btree_node *root, index_head *infor);
void Create_IndexTable(btree_node *root, index_head *infor);
void SetA(btree_node *root, index_head *infor);
key_type *Search_block(int x, btree_node *root, index_head *infor);


#endif
