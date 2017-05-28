#ifndef _STACK_H
#define _STACK_H

typedef struct {
	int elem[MAX_VERTEX_NUM];
	int top;
}stack;

void nstack_push(int e);
int nstack_pop();
int cstack_height();
void cstack_push(int e);
int cstack_pop();
#endif