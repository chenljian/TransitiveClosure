#include <stdlib.h>
#include <stdio.h>

#include "graph.h"

graph* create_graph(int verNum, int edgeNum) {

	graph* g = (graph*)malloc(sizeof(graph));
	g->vertex_num = verNum;
	g->edge_num = edgeNum;

	for (int i = 0; i < VERTEX_NUM; i++) {
		g->vertices[i].vertex = i;
		g->vertices[i].first_edge = NULL;
	}

	int count = 0;
	int v, w;
	srand((unsigned)time_t(0));
	while (count < edgeNum) {
		v = ((double)rand() / RAND_MAX) * verNum;
		w = ((double)rand() / RAND_MAX) * verNum;

		if (v != w && v < verNum && w < verNum && !edge_exist(g, v, w)) {
			edge_node* p = (edge_node*)malloc(sizeof(edge_node));
			p->vertex = w;
			p->next = g->vertices[v].first_edge;
			g->vertices[v].first_edge = p;
			count++;   
		}

	}
	return g;
}

bool edge_exist(graph* g, int src, int dst) {
	if (g == NULL)
		return false;
	if (src >= g->vertex_num || dst >= g->vertex_num)
		return false;

	edge_node* p = g->vertices[src].first_edge;
	while (p != NULL) {
		int w = p->vertex;
		if (w == dst)
			return true;

		p = p->next;
	}
	return false;
}