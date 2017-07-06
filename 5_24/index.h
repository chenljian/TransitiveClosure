typedef struct
{
	int x;
	int y;
	struct key_type *Forpointer;
}IndexTable;
typedef IndexTable AdjList[ARC_NUM];
AdjList table;
typedef struct L_Keypointer_Node
{  //用于Keypointer和KeyYpointer 分别把叶子块定位到原表的第某行
	int table_row;
	struct L_Keypointer_Node *next;
}L_Keypointer_Node, *L_Keypointer_List;
/*typedef struct L_Tuplepointer_Node
{
struct IndexTable *Tuplepointer;
struct L_Tuplepointer_Node *next;
}; //指向以当前块开头的索引表项（用于正向）*/

typedef struct key_type{
	int ivalue;
	L_Keypointer_List L_Keypointer;  //指向和当前块x相同的原表某些行 的链表的头结点（不为空）
	//	L_Keypointer_List L_KeyYpointer;  //指向和当前块y相同的原表某些行 的链表的头结点（不为空）
	//L_Tuplepointer_Node *L_Tuplepointer, *Tuplepointer_last; 
}key_type;
/*