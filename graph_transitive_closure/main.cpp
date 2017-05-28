#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <Windows.h>

#include "graph.h"
#include "component.h"
#include "stack.h"
#include "idList.h"

#define BUFFSIZE 50
#define TYPE_VERTEX 1
#define TYPE_COMPONENT 2
#define TYPE_PATH 3
graph* g = (graph*)malloc(sizeof(graph));
int visited[MAX_VERTEX_NUM];		//访问标志
int com_visited[MAX_VERTEX_NUM];
int root[MAX_VERTEX_NUM];			//结点所在的分量的根
int DFN[MAX_VERTEX_NUM];			//遍历结点的顺序
int C[MAX_VERTEX_NUM];				//结点所在的分量
int hsaved[MAX_VERTEX_NUM];			//高度
int nhsaved[MAX_VERTEX_NUM];
int in_degree[MAX_VERTEX_NUM];		//入度
int out_degree[MAX_VERTEX_NUM];		//出度
int com_in_degree[MAX_VERTEX_NUM];	//压缩后，图结点的入度

com_graph* com_g = (com_graph*)malloc(sizeof(com_graph));	//压缩图

stack nstack;
stack cstack;
stack path_stack;

component* c = (component*)malloc(sizeof(component));
component* suc = (component*)malloc(sizeof(component));

int order = 1;
int ccNum = 0;
idList* visited_id_list = (idList*)malloc(sizeof(idList));
idList* not_visited_id_list = (idList*)malloc(sizeof(idList));
void output_graph()
{
	for(int i = 0; i < g->vertex_num; i++)
	{
		printf("节点 %d 入度=%d 出度=%d :", i, in_degree[i], out_degree[i]);
		edge_node* edge_p = g->vertices[i].first_edge;
		while(edge_p)
		{
			printf("%d ", edge_p->vertex);
			edge_p = edge_p->next;
		}
		printf("\n");
	}
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
	char* path = "H:/craph_test.txt";
	init();
	visited_id_list->num = 0;
	not_visited_id_list->num = 0;
	readFile(path);
	//free(g);
	//g = create_graph(10000, 60000);
	output_graph();
	printf("start calculate componenet....\n");
	//start = clock();
	
	QueryPerformanceCounter(&litmp);
	//   获得初始值  
	QPart1 = litmp.QuadPart;
	for (int i = 0; i < g->vertex_num; i++) {
		if (visited[i] != 1)
			calculate_component(i);
	}
	//end = clock();
	QueryPerformanceCounter(&litmp);
	//   获得终止值  
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2-QPart1);
	dfTim = dfMinus / dfFreq;
	printf("component calculation cost %f seconds.\n", dfTim);
	
	partial_reset();
	printf("start calculate closure...\n");
	//start = clock();
	QueryPerformanceCounter(&litmp);
	//   获得初始值  
	QPart1 = litmp.QuadPart;
	for (int i = 0; i < g->vertex_num; i++) {
		if (visited[i] != 1)
			calculate_closure(i);
	}
	//end = clock();
	QueryPerformanceCounter(&litmp);
	//   获得终止值  
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2-QPart1);
	dfTim = dfMinus / dfFreq;
	printf("closure calculation cost %f seconds.\n", dfTim);
	//输出传递闭包
	for(int i = 0; i < g->vertex_num; i++)
	{
		printf("节点%d: ",i);
		output_closesure(C[i]);
		printf("\n");
	}

	output_graph();
	mid_init();
	printf("start compress graph...\n");
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;
	//开始压缩图
	for(int i = 0; i < g->vertex_num; i++)
	{
		if(visited[i] != 1 && !(in_degree[i] == 1 &&(out_degree[i] == 1 || out_degree == 0) ))
			compress_graph(i);
	}
	//Sleep(2000);
	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;
	dfMinus = (double)(QPart2-QPart1);
	dfTim = dfMinus / dfFreq;
	printf("compress graph cost %f seconds.\n", dfTim);

	reset_for_closesure();
	output_com_graph();
	printf("start calculate compressed graph closesure...\n");
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
	//输出传递闭包
	for(int i = 0; i < g->vertex_num; i++)
	{
		printf("node %d: ", i);
		output_closesure(i);
		printf("\n");
	}
	
	//printf("sum: %d\n", count());
	system("pause");
	free(g);
	free(c);
}

int count() {
	int sum = 0;
	for (int i = 0; i < g->vertex_num; i++) {
		int id = C[i];
		comp_node* p = c->c_list[id].first_vertex;
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
			comp_node* p = c->c_list[id].first_vertex;
			while (p != NULL) {
				printf("%d ", p->vertex);
				p = p->next;
			}
			printf("\n");
		}
	}
}
void calculate_closure(int v) {
	root[v] = v;
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
			if (DFN[root[v]] > DFN[root[w]])	//如果在同一个分量上
				root[v] = root[w];
		}
		else 
			cstack_push(C[w]);

		p = p->next;
	}

	if (root[v] == v) {
		//如果v是强连通分量的起点
		int vid = C[v];
		ccNum++;
		visited_id_list->num = 0;
		not_visited_id_list->num = 0;
		while (cstack_height() != hsaved[v]) {
			int id = cstack_pop();
		
			bool sign = false;
			comp_node* p = c->c_list[id].first_vertex;

			//之前的分量每一个顶点做循环
			while (p != NULL) {
				int w = p->vertex;
				int cid = C[w];
				if (id_exist(not_visited_id_list, cid)) {
					//插入到v所在的强连通分量
					comp_node* q = (comp_node*)malloc(sizeof(comp_node));
					q->vertex = w;
					q->next = c->c_list[vid].first_vertex;
					c->c_list[vid].first_vertex = q;
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
					q->next = c->c_list[vid].first_vertex;
					c->c_list[vid].first_vertex = q;

					
					not_visited_id_list->id[not_visited_id_list->num] = cid;
					not_visited_id_list->num++;
				}
				
				p = p->next;
			}

		}
		
	}

}

bool exist(int vid, int w) {
	comp_node* p = c->c_list[vid].first_vertex;

	while (p != NULL) {
		int v = p->vertex;
		if (v == w)
			return true;
		p = p->next;
	}
	return false;
}

void partial_reset() {
	for (int i = 0; i < MAX_VERTEX_NUM; i++) {
		root[i] = 0;
		DFN[i] = MAX_VERTEX_NUM;
		cstack.top = 0;	//这两行应该放到循环体外的
		order = 1;
		visited[i] = 0;
	}
}
void init() {
	for (int i = 0; i < MAX_VERTEX_NUM; i++) {
		visited[i] = 0;
		root[i] = MAX_VERTEX_NUM;
		DFN[i] = MAX_VERTEX_NUM;
		C[i] = -1;
		hsaved[i] = 0;
		nhsaved[i] = 0;
		nstack.top = 0;
		cstack.top = 0;
		g->vertices[i].first_edge = NULL;
		g->vertices[i].vertex = i;
		c->top = 0;
		c->c_list[i].first_vertex = NULL;
		c->c_list[i].id = i;
	}
	order = 0;
}


void mid_init() {
	for (int i = 0; i < MAX_VERTEX_NUM; i++) {
		visited[i] = 0;
		root[i] = MAX_VERTEX_NUM;
		DFN[i] = MAX_VERTEX_NUM;
		C[i] = -1;
		hsaved[i] = 0;
		nstack.top = 0;
		cstack.top = 0;
		c->top = 0;
		c->c_list[i].first_vertex = NULL;
		c->c_list[i].id = i;
		
	}
	order = 0;
	com_g->com_vertex_num = 0;
}

void calculate_component(int v) {
	root[v] = v;
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
			if (DFN[root[v]] > DFN[root[w]])
				root[v] = root[w];
		}

		p = p->next;
	}

	if (root[v] == v) {
		int id = c->top;
		c->top++;
		int w;
		do {
			w = nstack_pop();
			C[w] = id;
			comp_node* cnode = (comp_node*)malloc(sizeof(comp_node));
			cnode->vertex = w;
			cnode->next = c->c_list[id].first_vertex;
			c->c_list[id].first_vertex = cnode;
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
//返回第n个上面一个元素
int nstack_uper(int n)
{
	if(n == nstack.top)
		return 0;
	else
		return nstack.elem[n];
}
//返回第n个底下一个元素
int nstack_lower(int n)
{
	if(n <= 1)
		return 0;
	else
		return nstack.elem[n-2];
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
		printf("hsaved[v] = %d, cstack.height = %d", hsaved[v], cstack_height());
		int cid = cstack_pop();
		printf("当前cid的值是%d\n", cid);
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
int compress_graph(int v)
{
	root[v] = v;
	DFN[v] = order;
	order++;
	nstack_push(v);
	hsaved[v] = cstack_height();	//记录v在栈中的位置
	nhsaved[v] = nstack.top;
	printf("v = ：%d\n", v);
	visited[v] = 1;
	edge_node* edge_p = g->vertices[v].first_edge;
	//output_graph();
	while(edge_p != NULL)
	{
		int w = edge_p->vertex;
		if(visited[w] != 1)
			compress_graph(w);
		if(C[w] == -1)
		{
			//这里的C[w]可能是分量，也可能是路径
			if(DFN[root[v]] > DFN[root[w]])
				root[v] = root[w];
		}
		else
		{
			cstack_push(C[w]);
		}
		edge_p = edge_p->next;
	}

	if(root[v] == v)
	{
		int up = nstack_uper(nhsaved[v]);
		if(up == 0 || root[up] == up)
		{
			//说明这是路径上的点，判断要不要出栈构成路径
			int low = nstack_lower(nhsaved[v]); 
			if(low == 0
				|| out_degree[low] >= 2
				|| in_degree[v] != 1
				|| root[low] != low)
			{
				//这四种情况说明当前节点是路径起点，应该输出作为路径存储
				//com_g->com_vertex_num++;
				create_com_node(TYPE_PATH, v);
			}
		}
		else
		{
			//存储为强连通分量
			//com_g->com_vertex_num++;
			create_com_node(TYPE_COMPONENT, v);
		}
	}//if
	return 1;
}
int reset_for_closesure()
{
	//初始化suc

	//初始化c
	for(int i = 0; i < MAX_VERTEX_NUM; i++)
	{
		suc->c_list[i].first_vertex = NULL;
		c->c_list[i].first_vertex = 0;
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
		comp_p->next = c->c_list[v].first_vertex;
		c->c_list[v].first_vertex = comp_p;
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
			comp_p->next = c->c_list[v].first_vertex;
			c->c_list[edge_p->vertex].first_vertex = comp_p;
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
	comp_node* p = c->c_list[i].first_vertex;
	while(p)
	{
		printf("%d ",p->vertex);
		p = p->next;
	}
	return 1;
}