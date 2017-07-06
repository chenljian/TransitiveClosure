// 5_24.cpp : 定义控制台应用程序的入口点。 
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbool.h>
#include<windows.h>
#include <sys/timeb.h>
//#include <sys/mman.h>  
#include "mmap.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "btree.h"
#define BUFFSIZE 1024
#define _CRT_SECURE_NO_DEPRECATE

//calculate the time
struct timeval starttime, endtime;
double timeuse;
extern int alloc_count;
comp* c = (comp*)malloc(sizeof(comp)); 

//btree_node *root;
//index_head *infor;


/*
* 鎻掑叆鏃舵敞鎰忓弬鏁扮被鍨嬶紝0锛氭暣鍨嬶紝1锛氭诞鐐瑰瀷锛?锛氬瓧绗︿覆
*
*/
//#include "stdafx.h"
//#include "btree.h"
//#include "mmap.h" 
//#include<windows.h>
int alloc_count = 0;
btree_node *last_leaf_node;
btree_node *head_leaf;
int count = 1;
long int count1 = 0, count2 = 0;
long int btnode_count = 0, btrecord_count = 0, key_type_count = 0;
//make a empty leaf as the head leaf
void *alloc_add(int length, int type, index_head *infor, key_type *key) //allocate the free address
{
	block_node *next = (block_node *)infor->block[type].next, *add;
	//	if(type==0)printf("%d ",next); 
	if (next != NULL){
		if (type == 4){
			alloc_count += 5;
			//			printf("%d  ",next);
		}
		add = (block_node *)((long int)next + (long int)infor->base);
		//	if(next == 188){next = (block_node *)add;if(next->next==0)next->next=256;}
		next = (block_node *)add;
		infor->block[type].next = next->next;//delete the block
	}
	else{

		add = (block_node *)((long int)infor->offset + (long int)infor->base);
		infor->offset = infor->offset + (long int)length;
	}
	return add;
}
void initialize_keys(key_type *keys, key_type *key_input, int keys_type, key_type *tkey){
	if (keys_type == 0)//the key type is interger 
	{
		keys->ivalue = key_input->ivalue;

		keys->Keypointer = NULL;  //指向和当前块x相同的原表某些行 的链表的头结点（不为空）
		//keys->L_KeyYpointer = NULL;  //指向和当前块y相同的原表某些行 的链表的头结点（不为空）
		//keys->L_Tuplepointer = NULL;
		//keys->L_TupleYpointer = NULL;//五类指针  
		//	keys->L_TupleZpointer = NULL;
	}
}
int str_cmp(key_type *a, key_type *b, int keys_type){
	if (keys_type == 0){
		return a->ivalue - b->ivalue;
	}
}
int get_key_len(key_type *key, int type){//get the length of the key 
	return sizeof(key_type);
}
index_head *create_index(char *database_name, char *table_name, char *key_name, int key_type, int count, char *file_name){
	//create the index file; 
	int file_len, i;
	index_head *infor = NULL;
	strcpy(file_name, "../");
	strcat(file_name, database_name);
	strcat(file_name, "/");
	strcat(file_name, table_name);
	strcat(file_name, ".");
	strcat(file_name, key_name);
	strcat(file_name, ".idx");
	//file_name = "student.idx";
	file_len = count * 90;//    
	//int fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	fp = fopen(file_name, "rt+"); //allow read and write
	if (fp == NULL)
	{
		printf("open error\n");
		return 0;
	}
	fseek(fp, file_len - 1, SEEK_SET);
	fwrite("1", 4, 1, fp);
	fclose(fp);

	infor = (index_head *)open_mmap(file_name);
	infor->base = (void *)infor;
	infor->keys_type = key_type;//0:int 1:double 2:char
	infor->root = NULL;
	infor->node_count = 0;
	infor->offset = sizeof(index_head);//this is the last btree_node;
	for (i = 0; i< BLOCK_TYPE; i++)
		infor->block[i].next = NULL;
	infor->block[0].block_size = sizeof(btree_node);
	infor->block[1].block_size = btree_size * sizeof(void *);
	infor->block[2].block_size = (btree_size - 1) * sizeof(void *);
	infor->block[3].block_size = sizeof(btree_record);
	infor->block[4].block_size = 4;//the length of key
	return infor;
}
btree_node *find_leaf(btree_node *root, key_type *key, index_head *infor) //find the proper location for the key
{
	btree_node *nd;
	key_type **nd_keys;
	int i, count = 0;
	void **nd_pointers;
	if (root == NULL)
		return root;
	nd = root;
	while (!nd->is_leaf) {
		nd_keys = (key_type **)((long int)nd->keys + (long int)infor->base);
		nd_pointers = (void **)((long int)nd->pointers + (long int)infor->base);
		for (i = 0; i < nd->num_keys && str_cmp((key_type *)((long int)nd_keys[i] + (long int)infor->base), key, infor->keys_type) <= 0; i++, count++);
		nd = (btree_node *)((long int)nd_pointers[i] + (long int)infor->base);	 ///(btree_node *)nd->pointers[i]

	}
	return nd;
}
btree_record *make_new_btree_record(int value, index_head *infor, key_type *key) {
	btree_record *rec;
	rec = (btree_record *)alloc_add(sizeof(btree_record), 3, infor, key); ///(btree_record *)malloc(sizeof(btree_record))
	rec->value = value;
	return rec;
}
btree_node *make_new_btree_node(index_head *infor, key_type *key) {
	btree_node *nd;

	nd = (btree_node *)alloc_add(sizeof(btree_node), 0, infor, key);   ///(btree_node *)malloc(sizeof(btree_node))   
	nd->pointers = (void **)((long int)alloc_add(btree_size * sizeof(void *), 1, infor, key) - (long int)infor->base);//, key);///malloc(size * sizeof(void *))
	nd->keys = (key_type **)((long int)alloc_add((btree_size - 1) * sizeof(key_type *), 2, infor, key) - (long int)infor->base);//, key); ///malloc((size - 1) * sizeof(char *))
	nd->parent = NULL;
	nd->next_leaf_node = NULL;
	nd->num_keys = 0;
	nd->is_leaf = false;
	return nd;
}
btree_node *make_new_leaf(index_head *infor, key_type *key) {
	btree_node *leaf;
	leaf = make_new_btree_node(infor, key);
	leaf->is_leaf = true;
	last_leaf_node->next_leaf_node = leaf;
	last_leaf_node = leaf;
	return leaf;
}
btree_node *make_new_tree(key_type *key, int value, index_head *infor) {
	btree_node *root;
	btree_record *rec;
	root = make_new_leaf(infor, key);
	int type_len = get_key_len(key, infor->keys_type);
	void **root_pointers = (void **)((long int)root->pointers + (long int)infor->base);
	key_type **root_keys = (key_type **)((long int)root->keys + (long int)infor->base);
	rec = make_new_btree_record(value, infor, key);
	root_pointers[0] = (void *)((long int)rec - (long int)infor->base); //store the offset of rec
	root_keys[0] = (key_type *)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base);//, key); /// malloc(MAX_KEY_LEN) 
	key_type *temp = (key_type *)((long int)root_keys[0] + (long int)infor->base);

	initialize_keys(temp, key, infor->keys_type, key); /// strcpy(root->keys[0], key)
	root_pointers[btree_size - 1] = NULL; ///root->pointers[size-1]
	root->num_keys++;
	return root;
}
btree_node *make_new_root(btree_node *left, btree_node *right, key_type *key, index_head *infor, key_type *tkey) {
	btree_node *root;
	root = make_new_btree_node(infor, key);
	long int base = (long int)infor->base;
	int type_len = get_key_len(key, infor->keys_type);
	void **root_pointers = (void **)((long int)root->pointers + base);
	key_type **root_keys = (key_type **)((long int)root->keys + base);
	root_pointers[0] = (void *)((long int)left - base); /// root->pointers[0] = left; 
	root_pointers[1] = (void *)((long int)right - base); ///root->pointers[1] = right;
	root_keys[0] = (key_type *)((long int)alloc_add(type_len, 4, infor, key) - base); /// malloc(MAX_KEY_LEN)
	key_type *temp = (key_type *)((long int)root_keys[0] + base);
	initialize_keys(temp, key, infor->keys_type, tkey);
	root->num_keys++;
	left->parent = (btree_node *)((long int)root - base); ///root
	right->parent = (btree_node *)((long int)root - base); ///root
	return root;
}
void init_head_leaf(index_head *infor)
{
	key_type *head_leaf_key = (key_type *)malloc(MAX_KEY_LEN);
	head_leaf_key->ivalue = 0;
	head_leaf = make_new_btree_node(infor, head_leaf_key);  //make a empty leaf as the head leaf
	head_leaf->is_leaf = true;
	head_leaf->next_leaf_node = NULL;  //初始化空头叶子节点的next_leaf_node指针
	last_leaf_node = head_leaf;
}
btree_node *btree_insert(btree_node *root, key_type *key, int value, index_head *infor) {

	btree_record *rec;
	btree_node *leaf;
	int index, cond;

	leaf = find_leaf(root, key, infor);  //////////////////////////////////////////////////////////

	if (!leaf) {  // cannot find the leaf, the tree is empty 
		infor->node_count++;
		return  make_new_tree(key, value, infor);
	}

	key_type **leaf_keys = (key_type **)((long int)leaf->keys + (long int)infor->base);
	key_type *temp = (key_type *)((long int)leaf_keys[0] + (long int)infor->base);
	for (index = 0; index < leaf->num_keys && (cond = str_cmp((key_type *)((long int)leaf_keys[index] + (long int)infor->base), key, infor->keys_type)) < 0; index++);//strcmp(leaf->keys[index], key)

	if (cond == 0) {  // ignore duplicates,here we can change to store the duplicates by using the linklist
		return root;
	}
	infor->node_count++;
	rec = make_new_btree_record(value, infor, key);
	if (leaf->num_keys < btree_size - 1) {

		insert_into_leaf(leaf, index, key, rec, infor);
		return root;  // the root remains unchanged
	}
	return insert_into_leaf_after_splitting(root, leaf, index, key, rec, infor);//////////////////////////////////////////////////////////////
}

btree_node *insert_into_parent(btree_node *root, btree_node *left, btree_node *right, key_type *key, index_head *infor, key_type *tkey) {

	btree_node *parent;
	int index, i;
	parent = (btree_node *)left->parent;
	if (parent == NULL) {
		return make_new_root(left, right, key, infor, tkey);
	}
	parent = (btree_node *)((long int)left->parent + (long int)infor->base);  ///parent = left->parent;
	key_type **parent_pointers = (key_type **)((long int)parent->pointers + (long int)infor->base);
	for (index = 0; index < parent->num_keys && parent_pointers[index] != (key_type *)((long int)left - (long int)infor->base); index++);  ///parent->pointers[index] != left
	if (parent->num_keys < btree_size - 1) {
		insert_into_btree_node(parent, right, index, key, infor);
		return root;  // the root remains unchanged
	}
	return insert_into_btree_node_after_splitting(root, parent, right, index, key, infor);
}

void insert_into_btree_node(btree_node *nd, btree_node *right, int index, key_type *key, index_head *infor) {
	int i;
	void **nd_pointers = (void **)((long int)nd->pointers + (long int)infor->base);
	key_type **nd_keys = (key_type **)((long int)nd->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	for (i = nd->num_keys; i > index; i--) {
		nd_keys[i] = nd_keys[i - 1];  ///nd->keys[i] = nd->keys[i-1]
		nd_pointers[i + 1] = nd_pointers[i]; ///nd->pointers[i+1] = nd->pointers[i];
	}
	nd_keys[index] = (key_type *)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
	key_type *temp = (key_type *)((long int)nd_keys[index] + (long int)infor->base);
	initialize_keys(temp, key, infor->keys_type, key);
	nd_pointers[index + 1] = (void *)((long int)right - (long int)infor->base);  ///nd->pointers[index+1]
	nd->num_keys++;
}

btree_node *insert_into_btree_node_after_splitting(btree_node *root, btree_node *nd, btree_node *right, int index, key_type *key, index_head *infor) {
	int i, split;
	btree_node **temp_ps, *new_nd, *child;
	key_type **temp_ks, *new_key;
	void **nd_pointers = (void **)((long int)nd->pointers + (long int)infor->base);
	key_type **nd_keys = (key_type **)((long int)nd->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	void **new_nd_p;
	key_type **new_nd_k;
	temp_ps = (btree_node **)malloc((btree_size + 1) * sizeof(btree_node *));
	btnode_count++;
	temp_ks = (key_type **)malloc(btree_size * sizeof(key_type *));
	key_type_count++;
	for (i = 0; i < btree_size + 1; i++) {
		if (i == index + 1)
			temp_ps[i] = (btree_node *)((long int)right - (long int)infor->base);
		else if (i < index + 1)
			temp_ps[i] = (btree_node *)nd_pointers[i];  ///nd->pointers[i]
		else
			temp_ps[i] = (btree_node *)nd_pointers[i - 1];  ///nd->pointers[i-1]
	}
	for (i = 0; i < btree_size; i++) {
		if (i == index) {
			temp_ks[i] = (key_type *)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
			key_type *temp = (key_type *)((long int)temp_ks[i] + (long int)infor->base);
			initialize_keys(temp, key, infor->keys_type, key);
		}
		else if (i < index)
			temp_ks[i] = nd_keys[i];
		else
			temp_ks[i] = nd_keys[i - 1];
	}
	split = btree_size % 2 ? btree_size / 2 + 1 : btree_size / 2;  // split is #pointers
	nd->num_keys = split - 1;
	for (i = 0; i < split - 1; i++) {
		nd_pointers[i] = temp_ps[i];
		nd_keys[i] = temp_ks[i];
	}
	nd_pointers[i] = temp_ps[i];  // i == split - 1
	new_key = (key_type *)((long int)temp_ks[split - 1] + (long int)infor->base);

	new_nd = make_new_btree_node(infor, key);
	new_nd_p = (void **)((long int)new_nd->pointers + (long int)infor->base);
	new_nd_k = (key_type **)((long int)new_nd->keys + (long int)infor->base);
	new_nd->num_keys = btree_size - split;
	for (++i; i < btree_size; i++) {
		new_nd_p[i - split] = temp_ps[i];////
		new_nd_k[i - split] = temp_ks[i];

	}
	new_nd_p[i - split] = temp_ps[i];
	new_nd->parent = nd->parent;
	for (i = 0; i <= new_nd->num_keys; i++) {  //  #pointers == num_keys + 1
		child = (btree_node *)((long int)new_nd_p[i] + (long int)infor->base);
		child->parent = (btree_node *)((long int)new_nd - (long int)infor->base);
	}
	free(temp_ps);
	free(temp_ks);

	return insert_into_parent(root, nd, new_nd, new_key, infor, key);
}

void insert_into_leaf(btree_node *leaf, int index, key_type *key, btree_record *rec, index_head *infor) // insert the btree_node to the leaf without spliting
{
	int i;
	void **leaf_pointers = (void **)((long int)leaf->pointers + (long int)infor->base);
	key_type **leaf_keys = (key_type **)((long int)leaf->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	for (i = leaf->num_keys; i > index; i--) {
		leaf_keys[i] = leaf_keys[i - 1];
		leaf_pointers[i] = leaf_pointers[i - 1];
	}
	leaf_keys[index] = (key_type *)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base); /// malloc(MAX_KEY_LEN) 
	key_type *temp = (key_type *)((long int)leaf_keys[index] + (long int)infor->base);
	initialize_keys(temp, key, infor->keys_type, key);
	//printf("keys %d\n", key->ivalue);
	leaf_pointers[index] = (btree_record *)((long int)rec - (long int)infor->base);
	leaf->num_keys++;
}

btree_node *insert_into_leaf_after_splitting(btree_node *root, btree_node *leaf, int index, key_type *key, btree_record *rec, index_head *infor) {
	btree_node *new_leaf;
	btree_record **temp_ps;
	key_type **temp_ks, *new_key;
	int i, split;
	void **leaf_pointers = (void **)((long int*)leaf->pointers + (long int)infor->base);
	key_type **leaf_keys = (key_type **)((long int)leaf->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	void **new_leaf_pointers;
	key_type **new_leaf_keys;
	temp_ps = (btree_record **)malloc(btree_size * sizeof(btree_record *));
	btrecord_count++;
	temp_ks = (key_type **)malloc(btree_size * sizeof(key_type *));
	key_type_count++;
	for (i = 0; i < btree_size; i++) {
		if (i == index) {
			temp_ps[i] = rec;
			temp_ks[i] = (key_type *)alloc_add(type_len, 4, infor, key);
			initialize_keys(temp_ks[i], key, infor->keys_type, key);
		}
		else if (i < index) {
			temp_ps[i] = (btree_record *)((long int)leaf_pointers[i] + (long int)infor->base);
			temp_ks[i] = (key_type *)((long int)leaf_keys[i] + (long int)infor->base);
		}
		else {
			temp_ps[i] = (btree_record *)((long int)leaf_pointers[i - 1] + (long int)infor->base);
			temp_ks[i] = (key_type *)((long int)leaf_keys[i - 1] + (long int)infor->base);
		}
	}
	split = btree_size / 2;
	leaf->num_keys = split;
	for (i = 0; i < split; i++) {
		leaf_pointers[i] = (void **)((long int)temp_ps[i] - (long int)infor->base);
		leaf_keys[i] = (key_type *)((long int)temp_ks[i] - (long int)infor->base);
	}
	new_leaf = make_new_leaf(infor, key);
	new_leaf->num_keys = btree_size - split;
	new_leaf_pointers = (void **)((long int)new_leaf->pointers + (long int)infor->base);
	new_leaf_keys = (key_type **)((long int)new_leaf->keys + (long int)infor->base);
	for (; i < btree_size; i++) {
		new_leaf_pointers[i - split] = (void **)((long int)temp_ps[i] - (long int)infor->base);
		new_leaf_keys[i - split] = (key_type *)((long int)temp_ks[i] - (long int)infor->base);
	}
	new_leaf->parent = leaf->parent;
	new_leaf_pointers[btree_size - 1] = leaf_pointers[btree_size - 1];
	leaf_pointers[btree_size - 1] = (void **)((long int)new_leaf - (long int)infor->base);
	free(temp_ps);
	free(temp_ks);
	new_key = (key_type *)((long int)new_leaf_keys[0] + (long int)infor->base);
	return insert_into_parent(root, leaf, new_leaf, new_key, infor, key);/////////////////////////////////////////////////////
}
/********************************求解传递闭包*********************************/

int A[ARC_NUM + 1][2];  //存放边的表A
bool  reach[VERTEX_NUM + 1][VERTEX_NUM + 1] = { false };  //两点之间直接可达值为1 
IndexTable *L, *last;
//L_TupleYpointer_Node *TupleYpointer_last;
//L_TupleZpointer_Node *TupleZpointer_last;
L_Keypointer_Node *Keypointer_last;//, *KeyYpointer_last;
struct Map *Y;  //压缩图邻接表空表头
bool visit[VERTEX_NUM + 1] = { false };  //初始化每个点访问标记  
int CHeight[VERTEX_NUM + 1] = { 0 };
int ID[VERTEX_NUM + 1] = { 0 };  //每个点访问顺序
int Ltfl[VERTEX_NUM + 1] = { -1 }; //-1代表所有点现在还不属于某个分量
int id = 0;
int ar_num = 0;
int num = 0;

/***************栈操作******************/
int popNS(){
	if (ns.top >= 0) {
		int elem = ns.ns[ns.top];
		ns.top--;
		return elem;
	}
}

void pushNS(int elem) {
	ns.ns[ns.top + 1] = elem;
	ns.top++;
}

int popCS(){
	if (cs.top >= 0) {
		int elem = cs.cs[cs.top];
		cs.top--;
		return elem;
	}
}

void pushCS(int elem) {
	cs.cs[cs.top + 1] = elem;
	cs.top++;
}

int heightCS() {
	return cs.top;
}
/****************栈操作******************/


key_type *Search_block(int x, btree_node *root, index_head *infor)
{//查找指定顶点所在的块  （没问题）
	long int base = (long int)infor->base;
	int i;
	btree_node *p = root;
	key_type **p_keys, *temp;
	void **p_pointers; 
	while (!p->is_leaf)
	{
		p_pointers = (void **)((long int)p->pointers + (long int)base);
		p_keys = (key_type **)((long int)p->keys + (long int)base);
		for (i = 0; i < p->num_keys; i++)
		{

			temp = (key_type *)((long int)p_keys[i] + (long int)base);
			if (temp->ivalue == x)
			{
				i++;
				break;
			}
			if (temp->ivalue > x) break;
		}
		p = (btree_node *)((long int)p_pointers[i] + (long int)base);
	}
	if (p->is_leaf)
	{
		p_keys = (key_type **)((long int)p->keys + (long int)base);
		for (i = 0; i < p->num_keys; i++)
		{ 
			temp = (key_type *)((long int)p_keys[i] + (long int)base);
			if (temp->ivalue == x){   return temp; }
		}
	}
}
void SetIndexTable(btree_node *root, index_head *infor,char *path)
{//成功地把图从文件中读入表A (没问题)

	int r = 1, i, j;
	key_type *k=NULL;
	fp = fopen(path, "r");
	char str[15];    //存从文件中读入的一行字符串
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	i = 0; 
	//FILE* file = NULL;
	//file = fopen(pathStr, "r");

	char buff[BUFFSIZE];
	char* token = NULL;
	int v, w; 
	while (fgets(str, sizeof(str), fp))
	{
		i++;
		j = 1;
		while (str[j] != ',')
		{
			//A[r][0] = A[r][0] * 10 + (str[j] - 48);
			array_list[r].x = array_list[r].x * 10 + (str[j] - 48); 
		
			j++;
		}
		//printf("x:%d\n", array_list[r].x);
		j++;
		while (str[j] != '>')
		{
			array_list[r].y = array_list[r].y * 10 + (str[j] - 48); 
			//A[r][1] = A[r][1] * 10 + (str[j] - 48);
			j++;
		} 
		//printf("y:%d\n", array_list[r].y);
		k = Search_block(array_list[r].x, root, infor);
		if (!isexit_array_index[array_list[r].x])
		{ 
			isexit_array_index[array_list[r].x] = 1;
			arindex[ar_num].v = array_list[r].x; 
			arindex[ar_num].v_Forpointer = k; 
			ar_num++;
		}  
		array_list[r].Forpointer = Search_block(array_list[r].y, root, infor);  
	
		//array_list[r].Repointer = k; 

		L_Keypointer_Node *q;
		q = (L_Keypointer_Node *)malloc(sizeof(L_Keypointer_Node));
		q->array_row = r;  
		if (k->Keypointer == NULL)
		{ 
			q->next = NULL;
			k->Keypointer = q;
			k->Keypointer_last = q; 
		} 
		else
		{ 
			q->next = k->Keypointer_last->next;
			k->Keypointer_last->next = q;
			k->Keypointer_last = q; 
		} 
		/*L_Keypointer_List p;
		p = k->Keypointer;
			printf("%d ", r);
		while (p != NULL)
		{
			printf("%d %d  \n" ,array_list[p->array_row].x, array_list[p->array_row].y);
			p = p->next;
		}*/
	 	r++;

	} 

   /* printf("表A：\n");
	for (i = 1; i <= ARC_NUM; i++) 
		printf("<%d,%d,%d >\n", array_list[i].x, array_list[i].y, array_list[i].Forpointer->ivalue ); 
	printf("array_list: %d\n",ar_num);
	for (i = 0; i < ar_num; i++)
		printf("<%d %d>\n", arindex[i].v, arindex[i].v_Forpointer->ivalue);*/
} 

btree_node *test_insert(btree_node *root, index_head *infor, int s, int e) {
	key_type *key;
	int i, value;
	key = (key_type *)malloc(MAX_KEY_LEN);
	init_head_leaf(infor);
	for (i = s; i < e; i++) {
		srand(time(NULL));

		value = rand(); //printf("value=%d\n ", value);
		key->ivalue = i;  //10000000 + i;//鍒濆鍖杒ey锛屾暣鍨	/************	 *姝ゅ涓鸿皟鐢ㄦ彃鍏ユ帴鍙ｏ紝	 */  
		root = btree_insert(root, key, value, infor);
		infor->root = (btree_node *)((long int)root - (long int)infor->base);
	}
	free(key);
	return root;
}
bool isExistSucc(int elem, int id){
	EdgeNode* p = c->c[id].firstedge;
	
	while (p) {
		//if ((elem == 5) && (id == 1)) printf("yes\n");
		int v = p->adjvertex;
		//if ((elem == 5) && (id == 1)) printf("yes1\n");
		if (v == elem)
			return true;
		p = p->next;
	}
	return false;
}
bool isExistCS(int elem) {
	if (cs.top < 0)
		return false;
	int index = cs.top;
	while (index > 0) {
		if (cs.cs[index] == elem)
			return true;
		index--;
	}
	return false;
}
void tran_closure(key_type *kv, btree_node *root, index_head *infor)
{
	num++;
	
	//printf("kv:%d \n",kv->ivalue);
	int v = kv->ivalue;
	rot[v] = v;
	DFN[v] = index;
	index++;
	visited[v] = 1;
	cw[v] = -1;
	pushNS(v);
	hsvaed[v] = heightCS();

	//x = b+tree_find(v);//把之后的v全改成x
	L_Keypointer_List p = kv->Keypointer;
	//EdgeNode* p = g->adjlist[v].firstedge;  //这里不能用下标索引，改为用Ｂ＋树索引 
	while (p) {

		int w = array_list[p->array_row].y;
		if (visited[w] == 0) {
			tran_closure(array_list[p->array_row].Forpointer,root,infor);
		}
		if (cw[w] == -1) {
			if (DFN[rot[v]] > DFN[rot[w]])
				rot[v] = rot[w];
		}
		//这边貌似有问题
		else if (!isExistCS(cw[w])) {
			pushCS(cw[w]);
		}
		p = p->next;

	}
	 
	if (rot[v] == v)
	{    //v是强分量的根点
		
		int id = c->num;
		c->num++;
		int w;
		do {
			w = popNS();
			cw[w] = id;
			EdgeNode* p = (EdgeNode*)malloc(sizeof(EdgeNode));
			count++;
			p->adjvertex = w;
			p->next = NULL;
			p->flag = 1;
			c->c[id].no = id + 1;  //c[id].no是第id行这个强分量里的顶点数？
			p->next = c->c[id].firstedge;
			c->c[id].firstedge = p;
		} while (w != v);
		 
		while (heightCS() != hsvaed[v])
		{
			int x = popCS();
			EdgeNode* p = c->c[x].firstedge;

			while (p) {
				
				int v = p->adjvertex;
				//if (kv->ivalue == 2) printf("kv2到此 %d %d\n",v,id);
				isExistSucc(v, id);
				//if (kv->ivalue == 2) printf("kv2 4到此！\n");
				if (!isExistSucc(v, id)) {
					//if (kv->ivalue == 2) printf("kv2 2到此！\n");
					EdgeNode* q = (EdgeNode*)malloc(sizeof(EdgeNode));
					count++;
					q->adjvertex = v;
					q->flag = 2;
					//printf("v=%d\n", v);
					q->next = c->c[id].firstedge->next;
					c->c[id].firstedge->next = q;//应该改为尾插 ，头插是错误的
					

				}
				//if (kv->ivalue == 2) printf("kv2 3到此！\n");
				p = p->next;
			}//while

		}//while
	/*	EdgeNode *q = c->c[id].firstedge;
		while (q)
		{
			printf("%d ", q->adjvertex);
			q = q->next;
		}
		printf("\n"); */
	}//if

	//printf("finish kv:%d \n", kv->ivalue);
}
int _tmain(int argc, _TCHAR* argv[])
{
	//printf("sizeofkey_type:%d %d\n", sizeof(key_type), sizeof(key_type *));
	//clock_t start, end;
	c->num = 0;
	btree_node *root = NULL;
	index_head *infor;
	char file_name[50];
	int i = 0;
	ns.top = -1;
	cs.top = -1;
	for (int i = 0; i < VERTEX_NUM; i++) {
		c->c[i].no = i;
		c->c[i].firstedge = NULL; 
	}
	char* path = "testmap.txt";
	LARGE_INTEGER litmp;
	LONGLONG   QPart1, QPart2;
	double   dfMinus, dfFreq, dfTim, t1;
	QueryPerformanceFrequency(&litmp);
	//   获得计数器的时钟频率  
	dfFreq = (double)litmp.QuadPart;
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;

	/* *鍒涘缓绱㈠紩锛屽弬鏁板垎鍒负锛氳〃鍚嶏紝閿悕锛岀被鍨嬶紝璁板綍鏁帮紝鏂囦欢鍚嶏紙闇�鐢宠绌洪棿锛?	 */
	if ((infor = create_index("school", "student", "no", 0, 2000000, file_name)))
	{
		root = test_insert(root, infor, 0, VERTEX_NUM);//鎻掑叆 
		alloc_count = 0;
	} 
	SetIndexTable(root, infor,path);  // 设置数据结构成功 
	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2 - QPart1); 
	dfTim = dfMinus / dfFreq;
	printf("数据结构 cost %f ms.\n", dfTim * 1000);

	QueryPerformanceFrequency(&litmp);
	//   获得计数器的时钟频率  
	dfFreq = (double)litmp.QuadPart;
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;
	for (int i = 0; i <ar_num; i++)
	{
		if (visited[arindex[i].v] == 0) 
			tran_closure(arindex[i].v_Forpointer, root, infor); 
	} 
	/*QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2 - QPart1);
	dfTim = dfMinus / dfFreq;
	printf("计算联通分量 cost %f ms.\n", dfTim * 1000);

	QueryPerformanceFrequency(&litmp);
	//   获得计数器的时钟频率  
	dfFreq = (double)litmp.QuadPart;
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;*/
	EdgeNode *p, *q;
	int v, cun = 0;
	for (int i = 0; i < c->num; i++)
	{
		p = c->c[i].firstedge;

		//v = p->adjvertex;  
		while (p)
		{
			if (p->flag == 1)
			{
				q = c->c[i].firstedge;
				while (q)
				{
					if (p->adjvertex != q->adjvertex)
					{
						cun++;
						//printf("<%d,%d>\n", p->adjvertex, q->adjvertex);
					}
					q = q->next;
				}
			}
			p = p->next;
		}
	}
	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2 - QPart1);
	dfTim = dfMinus / dfFreq;
	printf("计算传递闭包 cost %f ms.\n", dfTim * 1000 );
	/*btree_node *p = head_leaf->next_leaf_node;
	key_type **p_keys, *temp;
	while (p)
	{
		p = p->next_leaf_node;
		for (i = 0; i < p->num_keys; i++)
		{
			p_keys = (key_type **)((long int)p->keys + (long int)infor->base);
			temp = (key_type *)((long int)p_keys[i] + (long int)infor->base);
			if (visited[temp->ivalue] == 0)
				tran_closure(temp, root, infor);

		}
		p = p->next_leaf_node;
	}*/
	
	//printf("num：%d %d\n", num,ar_num);
 
	 
	return 0;
}

