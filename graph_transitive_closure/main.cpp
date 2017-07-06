#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <Windows.h>

#include "graph.h"
#include "component.h"
#include "stack.h"
#include "idList.h"
#include "btree.h"
#include "mmap.h"


#define BUFFSIZE 50
#define TYPE_VERTEX 1
#define TYPE_COMPONENT 2
#define TYPE_PATH 3
graph* g = (graph*)malloc(sizeof(graph));

int com_visited[VERTEX_NUM];
int visited[VERTEX_NUM];
int rot[VERTEX_NUM];			//结点所在的分量的根
int DFN[VERTEX_NUM];			//遍历结点的顺序
int C[VERTEX_NUM];				//结点所在的分量
int hsaved[VERTEX_NUM];			//高度
int nhsaved[VERTEX_NUM];
int in_degree[VERTEX_NUM];		//入度
int out_degree[VERTEX_NUM];		//出度
int com_in_degree[VERTEX_NUM];	//压缩后，图结点的入度

 

int ar_num = 0;






com_graph* com_g = (com_graph*)malloc(sizeof(com_graph));	//压缩图

stack nstack;
stack cstack;
stack path_stack;

component* cp = (component*)malloc(sizeof(component));
component* suc = (component*)malloc(sizeof(component));

int order = 1;
int ccNum = 0;
idList* visited_id_list = (idList*)malloc(sizeof(idList));
idList* not_visited_id_list = (idList*)malloc(sizeof(idList));


btree_node *last_leaf_node;
btree_node *head_leaf;
int alloc_count = 0;
long int btnode_count = 0, btrecord_count = 0, key_type_count = 0;
FILE *filep;


void *alloc_add(int length, int type, index_head *infor, struct key_type *key) //allocate the free address
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
	filep = fopen(file_name, "rt+"); //allow read and write
	if (filep == NULL)
	{
		printf("open error\n");
		return 0;
	}
	fseek(filep, file_len - 1, SEEK_SET);
	fwrite("1", 4, 1, filep);
	fclose(filep);

	infor = (index_head *)open_mmap(file_name);
	infor->base = (void *)infor;
	infor->keys_type = key_type;//0:int 1:double 2:char
	infor->root = NULL;
	infor->node_count = 0;
	infor->offset = sizeof(index_head);//this is the last btree_node;
	for (i = 0; i < BLOCK_TYPE; i++)
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
			if (temp->ivalue == x){ return temp; }
		}
	}
}

void SetIndexTable(btree_node *root, index_head *infor, char *path)
{//成功地把图从文件中读入表A (没问题)

	int r = 1, i, j;
	key_type *k = NULL;
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

	key_type **p_keys, *temp;
	p_keys = (key_type **)((long int)root->keys + (long int)infor->base);
	temp = (key_type *)((long int)p_keys[0] + (long int)infor->base);

	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &v, &w);
		array_list[r].x = v;
		array_list[r].y = w;

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
			//printf("） %d %d\n", r, temp->ivalue);
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
//返回第n个上面一个元素
int nstack_uper(int n)
{
	if (n == nstack.top)
		return 0;
	else
		return nstack.elem[n];
}
//返回第n个底下一个元素
int nstack_lower(int n)
{
	if (n <= 1)
		return 0;
	else
		return nstack.elem[n - 2];
}
void create_com_node(int type, int v)
{
	int com_num = com_g->com_vertex_num;
	com_g->com_vertex_num++;

	(com_g->com_vertexs[com_num]).type = TYPE_PATH;
	com_g->com_vertexs[com_num].in_degree = 0;
	com_g->com_vertexs[com_num].out_degree = 0;
	com_g->com_vertexs[com_num].vertex_num = 0;
	com_g->com_vertexs[com_num].first_com_edge = NULL;
	vertex_node* node_p = (vertex_node*)malloc(sizeof(vertex_node));
	node_p->first_edge = NULL;
	node_p->vertex = 0;		//以下三个变量压缩图都不会用到
	node_p->in_degree = 0;
	node_p->out_degree = 0;
	com_g->com_vertexs[com_num].vertices = node_p;
	int w;
	do
	{
		w = nstack_pop();
		edge_node* edge_p = (edge_node*)malloc(sizeof(edge_node));
		edge_p->vertex = w;
		edge_p->next = node_p->first_edge;
		node_p->first_edge = edge_p;
		C[w] = com_num;
		com_g->com_vertexs[com_num].vertex_num++;
	} while (w != v);

	//构建压缩节点的边
	while (hsaved[v] != cstack_height())
	{
		//printf("hsaved[v] = %d, cstack.height = %d", hsaved[v], cstack_height());
		int cid = cstack_pop();
		//printf("当前cid的值是%d\n", cid);
		com_edge_node* com_edge_p = (com_edge_node*)malloc(sizeof(com_edge_node));
		com_edge_p->com_vertex = cid;
		com_edge_p->next = com_g->com_vertexs[com_num].first_com_edge;
		com_g->com_vertexs[com_num].first_com_edge = com_edge_p;
		com_g->edge_num++;
		com_g->com_vertexs[cid].in_degree++;
		com_g->com_vertexs[com_num].out_degree++;
	}
}

//压缩图
int compress_graph(key_type *v_p)
{
	int v = v_p->ivalue;
	rot[v] = v;
	DFN[v] = order;
	order++;
	nstack_push(v);
	hsaved[v] = cstack_height();	//记录v在栈中的位置
	nhsaved[v] = nstack.top;
	//printf("v = ：%d\n", v);
	visited[v] = 1;

	//
	L_Keypointer_List edge_p = v_p->Keypointer;

	//edge_node* edge_p = g->vertices[v].first_edge;
	//output_graph();
	while (edge_p != NULL)
	{
		int w = array_list[edge_p->array_row].y;
		//改变搜索方式
		if (visited[w] != 1)
			compress_graph(array_list[edge_p->array_row].Forpointer);
		if (C[w] == -1)
		{
			//这里的C[w]可能是分量，也可能是路径
			if (DFN[rot[v]] > DFN[rot[w]])
				rot[v] = rot[w];
		}
		else
		{
			cstack_push(C[w]);
		}

		//往下找
		edge_p = edge_p->next;
	}

	if (rot[v] == v)
	{
		int up = nstack_uper(nhsaved[v]);
		if (up == 0 || rot[up] == up)
		{
			//说明这是路径上的点，判断要不要出栈构成路径
			int low = nstack_lower(nhsaved[v]);
			if (low == 0
				|| out_degree[low] >= 2
				|| in_degree[v] != 1
				|| rot[low] != low)
			{
				//这四种情况说明当前节点是路径起点，应该输出作为路径存储
				create_com_node(TYPE_PATH, v);
				com_g->path_num++;
			}
		}
		else
		{
			//存储为强连通分量
			create_com_node(TYPE_COMPONENT, v);
			com_g->component_num++;
		}
	}//if
	return 1;
}

void output_com_graph()
{
	for(int i = 0; i < com_g->com_vertex_num; i++)
	{
		printf("节点 %d 入度=%d 出度=%d :", i, com_g->com_vertexs[i].in_degree, com_g->com_vertexs[i].out_degree);
		com_edge_node* com_edge_p = com_g->com_vertexs[i].first_com_edge;
		while(com_edge_p)
		{
			printf("%d ", com_edge_p->com_vertex);
			com_edge_p = com_edge_p->next;
		}
		edge_node* edge_p = com_g->com_vertexs[i].vertices->first_edge;
		printf(" 包含的结点：");
		while(edge_p)
		{
			printf("%d ", edge_p->vertex);
			edge_p = edge_p->next;
		}
		printf("\n");
	}

}
void main() {
	//clock_t start, end;
	LARGE_INTEGER litmp;
	LONGLONG   QPart1, QPart2;
	double   dfMinus, dfFreq, dfTim;
	QueryPerformanceFrequency(&litmp);
	//   获得计数器的时钟频率  
	dfFreq = (double)litmp.QuadPart;

	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;

	//添加的

	btree_node *root = NULL;
	index_head *infor;
	char file_name[50]; 
	for (int i = 0; i <= ARC_NUM; i++)
	{
		array_list[i].x = 0;
		array_list[i].y = 0;
		array_list[i].Forpointer = NULL;
	}
	for (int i = 0; i <= VERTEX_NUM; i++)
	{
		arindex[i].v = 0;
		arindex[i].v_Forpointer = NULL;
	}



	char* path = "0.01%-2500-100.txt";
	init();

	/*开始读文件+数据结构*/
	if ((infor = create_index("school", "student", "no", 0, 2000000, file_name)))
	{
		root = test_insert(root, infor, 0, VERTEX_NUM);//鎻掑叆 
		alloc_count = 0;
	}
	SetIndexTable(root, infor, path);  
	/*结束数据结构 */
	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2 - QPart1);
	dfTim = dfMinus / dfFreq;
	printf("数据结构 cost %f seconds.\n", dfTim);

	visited_id_list->num = 0;
	not_visited_id_list->num = 0;
	


	
	//start = clock();
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;
	


	/*开始计算传递闭包*/
	//开始压缩图
	for(int i = 0; i < ar_num; i++)
	{
		if (visited[arindex[i].v] != 1 && !(in_degree[arindex[i].v] == 1 && 
			(out_degree[arindex[i].v] == 1 || out_degree[arindex[i].v] == 0)))
			 
			compress_graph(arindex[i].v_Forpointer);
	}

	

	reset_for_closesure();
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;
	//开始在压缩图上计算闭包
	for(int i = 0; i < com_g->com_vertex_num; i++)
	{
		if(com_g->com_vertexs[i].in_degree == 0)
			calculate_com_graph_closesure(i);
	}

	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2-QPart1);
	dfTim = dfMinus / dfFreq;
	printf("calculate compressed graph closesure cost %f seconds.\n", dfTim);
	/*传递闭包计算完毕*/
	printf("路径：%d, 连通分量：%d\n", com_g->path_num, com_g->component_num);
	system("pause");
	free(g);
	free(cp);
	
}

int count() {
	int sum = 0;
	for (int i = 0; i < g->vertex_num; i++) {
		int id = C[i];
		comp_node* p = cp->c_list[id].first_vertex;
		while (p != NULL) {
			sum++;
			p = p->next;
		}
	}
	return sum;
}

void display() {
	for (int i = 0; i < g->vertex_num; i++) {
		int id = C[i];
		printf("\n%d:\n", i);
		if (id != -1) {
			comp_node* p = cp->c_list[id].first_vertex;
			while (p != NULL) {
				printf("%d ", p->vertex);
				p = p->next;
			}
			printf("\n");
		}
	}
}
void calculate_closure(int v) {
	rot[v] = v;
	DFN[v] = order;
	order++;
	hsaved[v] = cstack_height();
	visited[v] = 1;
	edge_node* p = g->vertices[v].first_edge;
	while (p != NULL) {
		int w = p->vertex;
		if (visited[w] != 1)
			calculate_closure(w);

		//判断v与w是否在同一个分量上
		if (C[w] == C[v]) {
			if (DFN[rot[v]] > DFN[rot[w]])	//如果在同一个分量上
				rot[v] = rot[w];
		}
		else 
			cstack_push(C[w]);

		p = p->next;
	}

	if (rot[v] == v) {
		//如果v是强连通分量的起点
		int vid = C[v];
		ccNum++;
		visited_id_list->num = 0;
		not_visited_id_list->num = 0;
		while (cstack_height() != hsaved[v]) {
			int id = cstack_pop();
		
			bool sign = false;
			comp_node* p = cp->c_list[id].first_vertex;

			//之前的分量每一个顶点做循环
			while (p != NULL) {
				int w = p->vertex;
				int cid = C[w];
				if (id_exist(not_visited_id_list, cid)) {
					//插入到v所在的强连通分量
					comp_node* q = (comp_node*)malloc(sizeof(comp_node));
					q->vertex = w;
					q->next = cp->c_list[vid].first_vertex;
					cp->c_list[vid].first_vertex = q;
				}
				else if (id_exist(visited_id_list, cid)) {
					

				}
				else if (exist(vid, w)) {
					//如果w在vid的强连通分量之中
					visited_id_list->id[visited_id_list->num] = cid;
					visited_id_list->num++;
				}
				else {
					comp_node* q = (comp_node*)malloc(sizeof(comp_node));
					q->vertex = w;
					q->next = cp->c_list[vid].first_vertex;
					cp->c_list[vid].first_vertex = q;

					
					not_visited_id_list->id[not_visited_id_list->num] = cid;
					not_visited_id_list->num++;
				}
				
				p = p->next;
			}

		}
		
	}

}

bool exist(int vid, int w) {
	comp_node* p = cp->c_list[vid].first_vertex;

	while (p != NULL) {
		int v = p->vertex;
		if (v == w)
			return true;
		p = p->next;
	}
	return false;
}

void partial_reset() {
	for (int i = 0; i < VERTEX_NUM; i++) {
		rot[i] = 0;
		DFN[i] = VERTEX_NUM;
		cstack.top = 0;	//这两行应该放到循环体外的
		order = 1;
		visited[i] = 0;
	}
}
void init() {
	for (int i = 0; i < VERTEX_NUM; i++) {
		visited[i] = 0;
		rot[i] = VERTEX_NUM;
		DFN[i] = VERTEX_NUM;
		C[i] = -1;
		hsaved[i] = 0;
		nhsaved[i] = 0;
		nstack.top = 0;
		cstack.top = 0;
		g->vertices[i].first_edge = NULL;
		g->vertices[i].vertex = i;
		cp->top = 0;
		cp->c_list[i].first_vertex = NULL;
		cp->c_list[i].id = i;
	}
	com_g->com_vertex_num = 0;
	com_g->path_num = 0;
	com_g->component_num = 0;
	order = 0;
}


void mid_init() {
	for (int i = 0; i < VERTEX_NUM; i++) {
		visited[i] = 0;
		rot[i] = VERTEX_NUM;
		DFN[i] = VERTEX_NUM;
		C[i] = -1;
		hsaved[i] = 0;
		nstack.top = 0;
		cstack.top = 0;
		cp->top = 0;
		cp->c_list[i].first_vertex = NULL;
		cp->c_list[i].id = i;
		
	}
	order = 0;
	com_g->com_vertex_num = 0;
}

void calculate_component(int v) {
	rot[v] = v;
	C[v] = -1;
	DFN[v] = order;
	order++;
	nstack_push(v);
	visited[v] = 1;
	edge_node* p = g->vertices[v].first_edge;
	while (p != NULL) {
		int w = p->vertex;
		if (visited[w] != 1)
			calculate_component(w);

		/*这个判断不理解，倘若判断w是否为已经访问过的结点，
		//则若w属于已经范文过的另一只分量的结点，而并不在nstack如何？
		//我知道了，这表示w没有被纳入某一强连通分量，那么这分为两种情况
		1是在栈中，二是未被访问*/
		if (C[w] == -1) {
			if (DFN[rot[v]] > DFN[rot[w]])
				rot[v] = rot[w];
		}

		p = p->next;
	}

	if (rot[v] == v) {
		int id = cp->top;
		cp->top++;
		int w;
		do {
			w = nstack_pop();
			C[w] = id;
			comp_node* cnode = (comp_node*)malloc(sizeof(comp_node));
			cnode->vertex = w;
			cnode->next = cp->c_list[id].first_vertex;
			cp->c_list[id].first_vertex = cnode;
		} while (w != v);
	}

}

void readFile(char* pathStr) {

	FILE* file = NULL;
	file = fopen(pathStr, "r");

	if (file == NULL) {
		printf("file: %s does not exist.", pathStr);
		system("psuse");
		exit(0);
	}

	char buff[BUFFSIZE];
	char* token = NULL;
	int v, w;
	g->vertex_num = 10;
	g->edge_num = 0;
	while (fgets(buff, BUFFSIZE, file)) {

		token = strtok(buff, "<,>");
		v = atoi(token);
		token = strtok(NULL, "<,>");
		w = atoi(token);

		edge_node*  p = (edge_node*)malloc(sizeof(edge_node));
		p->vertex = w;
		p->next = g->vertices[v].first_edge;
		g->vertices[v].vertex = v;
		g->vertices[v].first_edge = p;
		g->edge_num++;

		//记录出入度
		in_degree[w]++;
		out_degree[v]++;
	}
	
}

void nstack_push(int e) {
	nstack.elem[nstack.top] = e;
	nstack.top++;
}
int nstack_pop() {
	int e = nstack.elem[nstack.top - 1];
	nstack.top--;
	return e;
}

int cstack_height() {
	return cstack.top;
}
void cstack_push(int e) {
	cstack.elem[cstack.top] = e;
	cstack.top++;
}
int cstack_pop() {
	int e = cstack.elem[cstack.top - 1];
	cstack.top--;
	return e;
}

int reset_for_closesure()
{
	//初始化suc

	//初始化c
	for(int i = 0; i < VERTEX_NUM; i++)
	{
		suc->c_list[i].first_vertex = NULL;
		cp->c_list[i].first_vertex = 0;
		visited[i] = 0;\
		hsaved[i] = 0;
	}
	return 1;
}

//把cid加入v的压缩节点后继
int add_com_suc(int v, int cid)
{
	comp_node* comp_p = (comp_node*)malloc(sizeof(comp_node));
	comp_p->vertex = cid;
	comp_p->next = suc->c_list[v].first_vertex;
	suc->c_list[v].first_vertex = comp_p;
	return 1;
}

//把压缩节点com_node添加到v的c_list中
int add_suc_to_node(int v, int com_node)
{
	edge_node* edge_p = com_g->com_vertexs[com_node].vertices->first_edge;
	while(edge_p)
	{
		comp_node* comp_p = (comp_node*)malloc(sizeof(comp_node));
		comp_p->vertex = edge_p->vertex;
		comp_p->next = cp->c_list[v].first_vertex;
		cp->c_list[v].first_vertex = comp_p;
		edge_p = edge_p->next;
	}
	return 1;
}
int calculate_com_graph_closesure(int v)
{
	hsaved[v] = cstack_height();
	visited[v] = 1;
	com_edge_node* com_edge_p = com_g->com_vertexs[v].first_com_edge;
	while (com_edge_p)
	{
		if(com_visited[com_edge_p->com_vertex]!= 1)
			calculate_com_graph_closesure(com_edge_p->com_vertex);
		
		cstack_push(com_edge_p->com_vertex);
		com_edge_p = com_edge_p->next; 
	}
	
	visited_id_list->num = 0;
	//把cstack中的压缩节点的后继加入到本节点后继
	while (cstack_height() != hsaved[v])
	{
		int cid = cstack_pop();
		//这个节点加入当前节点的后继
		add_com_suc(v, cid);
		//这个节点的后继加入当前节点的后继
		comp_node* comp_p = suc->c_list[cid].first_vertex;
		while (comp_p)
		{
			if(!id_exist(visited_id_list, comp_p->vertex))
			{
				add_com_suc(v, comp_p->vertex);
				visited_id_list->num++;
				visited_id_list->id[visited_id_list->num] = comp_p->vertex;
			}
			comp_p = comp_p->next;
		}
	}
	//根据suc求当前压缩节点的每一个节点的后继
	com_vertex_node* com_node_p = &com_g->com_vertexs[v];
	edge_node* edge_p = com_node_p->vertices->first_edge;
	while(edge_p)
	{
		//压缩节点里面先求
		edge_node* suc_node;
		if(com_node_p->type == TYPE_PATH)
			suc_node = edge_p->next;
		else
			suc_node = com_node_p->vertices->first_edge;
		while(suc_node)
		{
			comp_node* comp_p = (comp_node*)malloc(sizeof(comp_node));
			comp_p->vertex = suc_node->vertex;
			comp_p->next = cp->c_list[v].first_vertex;
			cp->c_list[edge_p->vertex].first_vertex = comp_p;
			suc_node = suc_node->next;
		}
		//求压缩节点后面的节点
		comp_node* comp_p = suc->c_list[v].first_vertex;
		while(comp_p)
		{
			add_suc_to_node(edge_p->vertex, comp_p->vertex);
			comp_p = comp_p->next;
		}
		edge_p = edge_p->next; 
	}
	return 1;
}
int output_closesure(int i)
{
	comp_node* p = cp->c_list[i].first_vertex;
	while(p)
	{
		printf("%d ",p->vertex);
		p = p->next;
	}
	return 1;
}