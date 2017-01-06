/********************************************************************
              Copyright 1993 University of Bern              
     
                   All Rights Reserved
 
Permission to use this software and its documentation for any purpose
 and without fee is hereby granted, provided that the above copyright 
notice appear in all copies and that both that copyright notice and this 
permission notice appear in supporting documentation. Do not distribute 
without specific, written prior permission.  
                                                  
Author: Bruno T. Messmer	                       
Email:  messmer@iam.unibe.ch	                   
********************************************************************/

#ifndef _adjazenzmatrix_h
#define _adjazenzmatrix_h

#include <stdio.h>
#include <string.h>
#include "gmt_assert.h"
#include "Randomizer.h"
//#include "AttributeClass.h"
#include "AttControl.h"
#include "Token.h"


using namespace std;


struct Tsnode{
  List parts;
  List conns;
  List connsDir;
  AttId at;
  double err;
};


class AdjazenzMatrix{
  friend class Graph;
  
 public:
  
  AdjazenzMatrix();
  ~AdjazenzMatrix();
  
  AdjazenzMatrix(AdjazenzMatrix& am);
  AdjazenzMatrix operator=(AdjazenzMatrix& am);
  
  void doOnlyIdenticalEdges();
  
  void createNow(int directed);
  void clear();

  int isDirected();
  void create(int dimension,int directed);
  void randomGraph(int dimension,int directed,int labels,int expect,int varianz,AdjazenzMatrix* subgraph);
 void randomGraph(int dimension,int directed,int labels,int expect,int varianz,AdjazenzMatrix* subgraph,int labelOffset);


  void randomMerge(int labels,int expect,int varianz,AdjazenzMatrix* subgraph);
  double transformGraph(int eps,double* types,AttributeClass *ATT,int startn,int endn);
  
  void setEdge(int x,int y,int edge);
  void setNodeAttributeId(int x,AttId &att);
  AttId getNodeAttributeId(int x);
  void setEdgeAttributeId(int x,AttId &att,int n1,int n2);
  AttId getEdgeAttributeId(int x,int* n1,int *n2);
  
  void nextSuperEdge(int,int);

  void setSuperNode(List& parts,List& conns, List& connsDir, AttId &att, double err);
  int numberOfSuperNodes();

  void deleteSuperNodes();

  int inSuper(int x,int y);

  void switchAttributeWithLabel(AttributeClass * ATT);

  int isEdge(int x,int y);
  int edgeDegree(int x,int y);
  int degree(int x);
  int numberOfVertices();
  int numberOfEdges();
  void concatMatrix(AdjazenzMatrix* F);

  void read(char* name);
  void write(char* name);

  int writeToFile(char* gname,int number,AttributeClass* Attr,FILE *file);
  int readFromFile(AttributeClass* Attr,FILE *file);
  int readFromFile(AttributeClass* Attr,char* name);
  

  void initNext(int x,int y);
  void initNext(int x);
  int getnext(int* in,int* out, AttId* at);
  AttId isNextEdgeAttId(int* edg);

  void expandableEdges(int x,int y,int& ex,int &ey);
  void expand(int x,int dir,List& lx);

  Token* orderCoherent();

  void setName(char* n);
  char* Name();
  void setNumber(int n);
  int Number();

  void createDef(char* name, int labels,double ins,double var,double sub);
  void createDef(char* name, int labels,double ins,double var,double sub,int* bounds,int blimit);

  int connectSingleTrees(int labels);


  int COMPLEX_ATTRIBUTES;

  void setGrain(int i);
  int isGrain;

  void* owner;

// STUDIES variable
  int edge_access;

  int edgesAllocated;

//private:

  int IDENTICAL_EDGES_ONLY;

  struct EdgeList{
    int edge;
    EdgeList* next;
  };

  struct EdgeInfo{
    AttId edgeAtt;
    int n1,n2;
  };


  int BothSides;
  int temp_Dimension,real_Dimension;
  int temp_edgeDimension,real_edgeDimension;
  
  int Dimension,edgeDimension,Directed;
 
  EdgeList** matrix;
  AttId* NodeAttributes;
  EdgeInfo* EdgeAttributes;
  EdgeList *NextPtr;

  int NextIndex,NextArg;

  char* name;
  int number;


  int *Bounds;
  int Blimit;
  int LabelOffset;


  int expandable_edges_flag;
  
  List SuperNodes;

  int currX,currY;
  int superConnX,superConnY;
  int SuperEdge;

  Tsnode* g_snx,*g_sny;

};

  
#endif
