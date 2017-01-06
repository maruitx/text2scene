#ifndef _MetaGraph
#define _MetaGraph

#include "Token.h"
#include "ComNet.h"
#include "Graph.h"
#include "List.h"
class NNodeType;

class MetaGraph{

 public:
  MetaGraph();
  ~MetaGraph();

  int represented;
  Graph* graph;
  Token* original;

  NNodeType* Node;
  int node;
  int type;

  int referenced;
  static void deleteThis(MetaGraph* pt); 
  
}; 


class In_MetaGraph{

 public:
  MetaGraph* MyGraph;
  Graph* graph;
  
  List parts;  // contains a list of In_MetaGraphs
  int vertex;
  
  Token *tok;


  In_MetaGraph();
  ~In_MetaGraph();

  void init(MetaGraph* M,Graph* G);
  void init(int v,MetaGraph* M,Graph* G);
  void add(In_MetaGraph *M);
  void getVertices(List& l);
  int connectedTo(In_MetaGraph* M);
  void clear();
  Graph* subGraph();
  int size();
  int innerDegree();
  int degree(int &inner);
  

};

List* createMetaGraph(List& L,int type);
List* compress(List* G);
In_MetaGraph* hierarchy(In_MetaGraph* M);



#endif
