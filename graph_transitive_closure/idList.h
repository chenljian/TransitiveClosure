#ifndef _IDLIST_H
#define _IDLIST_H
#include "graph.h"

typedef struct {
	int id[MAX_VERTEX_NUM];
	int num;

}idList;

void idListReset(idList* list);
bool id_exist(idList* list, int id);
#endif 
