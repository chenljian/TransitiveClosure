#include"btree.h"




btree_node *last_leaf_node;
btree_node *head_leaf;
int alloc_count = 0;
long int btnode_count = 0, btrecord_count = 0, key_type_count = 0;
FILE *filep;
int index = 1;


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