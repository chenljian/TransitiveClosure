#ifndef _GRAPH_H
#define _GRAPH_H

#define VERTEX_NUM 5000

typedef struct edge {
	int vertex;
	edge* next;
}edge_node;

typedef struct {
	int vertex;
	edge_node* first_edge;
	int in_degree;	//入度（新增）
	int out_degree;
}vertex_node;

typedef vertex_node vertex_list[VERTEX_NUM];

typedef struct {
	vertex_list vertices;
	int vertex_num;
	int edge_num;
}graph;

//以下是压缩图的数据结构
typedef struct com_edge{
	int com_vertex;
	com_edge* next;
}com_edge_node;

typedef struct{
	int type;
	vertex_node* vertices;
	int vertex_num;
	com_edge* first_com_edge;
	int in_degree;	//入度
	int out_degree;	
	
}com_vertex_node;


typedef com_vertex_node com_vertex_list[VERTEX_NUM];

typedef struct{
	com_vertex_list com_vertexs;
	int com_vertex_num;
	int edge_num;
	int path_num;
	int component_num;
}com_graph;


graph* create_graph(int verNum, int edgeNum);
bool edge_exist(graph* g, int src, int dst);
void calculate_component(int v);
void readFile(char* pathStr);
void init();
void mid_init();
void calculate_closure(int v);
void partial_reset();
bool exist(int vid, int w);
void display();
int count();

int calculate_com_graph_closesure(int);
int reset_for_closesure();
int output_closesure(int);
#endif
