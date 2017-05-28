// COMP_TC.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"

#include <math.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include "Graph.h"
#define BUFFSIZE 1024

AdjGraph* g = (AdjGraph*)malloc(sizeof(AdjGraph));

void graph(int vertexNum, int edgeNum);
int index = 1;
int hsvaed[MAXVERTEX];
int visited[MAXVERTEX];
void init();

int _tmain(int argc, _TCHAR* argv[])
{
	clock_t start, end;
	init();
	/*Graph.txt stores the edges in the format of <v,w>. v,w represent the vertex.*/
	char* path = "C:\\Graph.txt";
	
	printf("%s\n", "start creating graph.");
	readFile(path); // graph g is created based on the edges from above file.
	printf("%s\n", "graph created.");


	start = clock();

	for (int v = 0; v < g->n; v++) {
		if (visited[v] == 0)
			tc(v);  // calculate the graph closure.
	}
	end = clock();

	printf("seconds: %f\n", (double)(end - start)/ CLOCKS_PER_SEC);
	
	

	return 0;
}

void tc(int v){
	root[v] = v;
	DFN[v] = index;
	index++;
	visited[v] = 1;
	Cw[v] = -1;
	pushNS(v);
	hsvaed[v] = heightCS();

	EdgeNode* p = g->adjlist[v].firstedge;

	while (p) {
		int w = p->adjvertex;
		if (visited[w] == 0) {
			tc(w);
		}

		if (Cw[w] == -1) {
			if (DFN[root[v]] > DFN[root[w]])
				root[v] = root[w];
		}
		else if (!isExistCS(Cw[w])) {
			pushCS(Cw[w]);
		}

		p = p->next;
	}

	if (root[v] == v) {
		int id = c->num;
		c->num++;
		int w;
		do {
			w = popNS();
			Cw[w] = id;
			EdgeNode* p = (EdgeNode*)malloc(sizeof(EdgeNode));
			p->adjvertex = w;
			p->next = NULL;
			c->c[id].no = id+1;
			p->next = c->c[id].firstedge;
			c->c[id].firstedge = p;
		} while (w != v);

		while (heightCS() != hsvaed[v]) {
			int x = popCS();
			EdgeNode* p = c->c[x].firstedge;

			while (p) {
				int v = p->adjvertex;

				if (!isExistSucc(v, id)) {
					EdgeNode* q = (EdgeNode*)malloc(sizeof(EdgeNode));
					q->adjvertex = v;
					q->next = c->c[id].firstedge;
					c->c[id].firstedge = q;
				}
				p = p->next;
			}

		}
	}

}

bool isExistSucc(int elem, int id){
	EdgeNode* p = c->c[id].firstedge;

	while (p) {
		int v = p->adjvertex;
		if (v == elem)
			return true;
		p = p->next;
	}

	return false;
}

void readFile(char* pathStr){
	FILE* file = NULL;
	file = fopen(pathStr, "r");

	char buff[BUFFSIZE];
	char* token = NULL;
	int v, w;
	g->n = 100000;
	g->e = 0;
	while (fgets(buff, BUFFSIZE, file)) {
		
		token = strtok(buff, "<,>");
		v = atoi(token);
		token = strtok(NULL, "<,>");
		w = atoi(token);

		EdgeNode*  p = (EdgeNode*)malloc(sizeof(EdgeNode));
		p->adjvertex = w;
		p->next = g->adjlist[v].firstedge;
		g->adjlist[v].vertex = v;
		g->adjlist[v].firstedge = p;
		g->e++;
	}
}

/*create a graph*/
void graph(int vertexNum, int edgeNum){
	g->n = vertexNum;
	g->e = edgeNum;
	
	srand((unsigned)time(0));
	
	int i, j;
	bool isExist = false;
	EdgeNode* s;
	for (int count = 0; count < edgeNum;) {
		i = rand() % vertexNum;
		j = rand() % vertexNum;

		if (i != j) {
			EdgeNode* p = g->adjlist[i].firstedge;
			while (p) {

				int vertex = p->adjvertex;

				if (vertex == j) {
					isExist = true;
					break;

				}

				p = p->next;

			}

			if (!isExist) {

				s = (EdgeNode*)malloc(sizeof(EdgeNode));
				s->adjvertex = j;
				s->next = g->adjlist[i].firstedge;
				g->adjlist[i].firstedge = s;
				count++;

			}
		}
		
		isExist = false;
	}



}

void init() {
	/*init the graph.*/
	for (int i = 0; i < MAXVERTEX; i++) {
		g->adjlist[i].vertex = i;
		g->adjlist[i].firstedge = NULL;
		g->n = 0;
		g->e = 0;
	}

	ns.top = -1;
	cs.top = -1;
	
	for (int i = 0; i < MAXVERTEX; i++) {
		Cw[i] = -1;
		hsvaed[i] = -1;
		visited[i] = 0;
	}

	c->num = 0;

	for (int i = 0; i < MAXVERTEX; i++) {
		c->c[i].no = i;
		c->c[i].firstedge = NULL;
	}
}