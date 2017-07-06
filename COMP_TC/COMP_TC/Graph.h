
#ifndef _Graph_h_
#define _Graph_h_


#define MAXVERTEX 4

typedef struct node
{
	int adjvertex;
	node* next;

}EdgeNode;

typedef struct
{
	int vertex;
	EdgeNode* firstedge;
}VertexNode;

typedef VertexNode AdjList[MAXVERTEX];

typedef struct
{
	AdjList adjlist;
	int n;	//图的顶点个数
	int e;	//图的边数
}AdjGraph;

typedef struct
{
	int ns[MAXVERTEX];
	int top;
}nstack;

typedef  struct
{
	int cs[MAXVERTEX];
	int top;
}cstack;

typedef struct {
	int no;
	EdgeNode* firstedge;
}CNode;

typedef CNode component[MAXVERTEX];

typedef struct {
	int num;
	component c;
}comp;


nstack ns;
cstack cs;
int Cw[MAXVERTEX];
int root[MAXVERTEX];
int DFN[MAXVERTEX];
comp* c = (comp*)malloc(sizeof(comp));

void tc(int v);
bool isExistSucc(int elem, int id);
int popNS(){
	if (ns.top >= 0) {
		int elem = ns.ns[ns.top];
		ns.top--;
		return elem;
	}
}

void pushNS(int elem) {
	ns.ns[ns.top+1] = elem;
	ns.top++;
}

int popCS(){
	if (cs.top >= 0) {
		int elem = cs.cs[cs.top];
		cs.top--;
		return elem;
	}
}

void pushCS(int elem) {
	cs.cs[cs.top + 1] = elem;
	cs.top++;
}

int heightCS() {
	return cs.top;
}

bool isExistCS(int elem, int height){
	if (cs.top < 0)
		return false;
	int index = cs.top;
	while (index > height) {	
		if (cs.cs[index] == elem)
			return true;
		index--;
	}
	return false;
}

void readFile(char* pathStr);

#endif