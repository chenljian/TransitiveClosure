#include "idList.h"
#include <stdlib.h>
void idListReset(idList* list) {
	int i = 0;

	for (; i < list->num; i++) {
		list->id[i] = -1;
	}
}

bool id_exist(idList* list, int id) {
	int i = 0;
	for (; i < list->num; i++) {
		if (list->id[i] == id)
			return true;
	}
	return false;
}