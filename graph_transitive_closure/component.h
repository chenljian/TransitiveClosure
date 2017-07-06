#ifndef _COMPONENT_H
#define _COMPONENT_H


typedef struct comp_node{
	int vertex;
	comp_node* next;
}comp_node;

typedef struct comp {
	int id;
	comp_node* first_vertex;
}comp;

typedef comp comp_list[VERTEX_NUM];

typedef struct {
	int top;
	comp_list c_list;
}component;

#endif