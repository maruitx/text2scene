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


#ifndef _graphclass
#define _graphclass

#include "AdjazenzMatrix.h"
#include "AttControl.h"
#include "List.h"


extern AttributeClass ATT_object;

class Graph;

struct ModelDataBaseType{
  Graph *G;
  int key;
};


struct Edit_type{
  int type;  // 0 edge insert; 1 edge delete; 2 vertex attribute substitute; 3 edge substitute
  int x;
  int y;
  int label;
  double *values;
  int n;
  double cost;
};


struct InstanceData{
  int* model;
  int* image;
  double *error;
  int dim;
  int modelNr;
  int modelKey;
  double totalError;
  int numberOfSubstitutions;
};

void attribute_definition(char* name);


class Graph{

 public:

  Graph();
  ~Graph();
  
  Graph* copy();


  void set(int x, int label, double* values, int n);
  void set(int x, int label, double* values, int n, void* complex);
  void resetAttributes(int x, int label, double* values, int n);
  void setEdge(int x, int y, int e, int l, double* value,int n);
  void resetEdgeAttributes(int edg, int label, double* values, int n);

  void discard();
 
  void done(int directed); 

  void setName(const char* name);
  char* Name();

  int read(char* name);
  int write(char* name);
 
  int maximumVertex();

  int numberOfVertices();
  int numberOfEdges();

  /***** ACCES VIA ORIGINAL NUMBER ******/

  void get(int x, int& label, double *&values, int &n);
  int getNumberOfEdges(int v1,int v2);
  int getIndex(int v);
  int getEdgesAdjacentTo(int x,int ind ,int &to_vertex,int& label, double* &values, int &n,int &dir);
  int getEdgesAdjacentTo(int x,int ind ,int &to_vertex,int& label, double* &values, int &n);
  void getEdge(int edge,int &n1,int &n2, int& label, double* &values, int &n);
  int getEdgeNR(int x);
  void* getComplex(int v, int& desc);

  /*************************************/

  /***** ACCES VIA INDEX NUMBER   ******/

  int getI(int index, int& label, double *&values, int &n);
  int getI(int index);
  void* getIcomplex(int index, int& desc);
  int getEdgeNR(int x,int y);
  int getEdgeId(int index, int &n1,int &n2,int& label, double*& values, int &n);
  int getEdgesBetween(int x,int y, int ind ,int& label, double* &values, int &n);
  
  int getSuperId(int x, int& label,double *& values,int &n);
  void generateSuperNodes(double threshold,double weight)  ;

/************************************/

  
  void generateDistortions(SortedList& distlist,double max_error);
  Graph* applyEdits(Edit_type* E);


  void expandableEdges(int f);

  void defaultKeys();
  void doOnlyIdenticalEdges();
  void dump();

  void setVisibility(int v,int h);

  Graph* getSubgraph(List& vertices);
  void deleteSuperNodes();
  int visible(int v);

  Graph* getVisibleGraph();

  List* getMinimalCycles(int length);


    static int basic_steps;
  static int loadedDefinition;
  //static AttributeClass ATT_object;

  AdjazenzMatrix AM;
  SortedList Vertices;
  SortedList Edges;    

  int MaximumVertex;
  int xVertices;
  int xEdges;

  int Done;
  char name[256];

  SortedList VISIBILITY;

  void* user_data;

};

#endif
