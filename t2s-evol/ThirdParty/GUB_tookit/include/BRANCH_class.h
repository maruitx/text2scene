/********************************************************************
              Copyright 1994 University of Bern              
     
                   All Rights Reserved
 
Permission to use this software and its documentation for any purpose 
and without fee is hereby granted, provided that the above copyright 
notice appear in all copies and that both that copyright notice and this 
permission notice appear in supporting documentation. Do not distribute 
without specific, written prior permission.  
                                                  
Author: Bruno T. Messmer	                       
Email:  messmer@iam.unibe.ch	                   
********************************************************************/


#ifndef _BRANCH_class
#define _BRANCH_class
#include <stdio.h>
#include "Graph.h"
#include "AttObject.h"
#include "AttControl.h"

extern AttributeClass ATT_object;

#ifndef INT_TYPE
#define INT_TYPE char
#endif

#if 0
struct BIN_class{
  BIN_class* succ[2];  
  List* successors;
};


#else
struct BIN_class{

  void* succ[2];  
  List* successors;
};
#endif

struct Next_class{
  int model;
  INT_TYPE vertex;
  int next;
};


struct EDGE_nextclass{
  int model;
  INT_TYPE edge;
  EDGE_nextclass* next;
  BIN_class* son;
};


class BRANCH_class{

 public:
  
  BRANCH_class();
  ~BRANCH_class();
  
  int test(int* edges,int vertex,AdjazenzMatrix* AM);
  int add(int* edges,int successor,int step,int vertex, AdjazenzMatrix* AM,int num);
  
  BIN_class* testEdge(AttId* at,int edge,void** succ);

    int write(FILE* file);
    int writeNode(BIN_class* node,FILE* file);
    int read(FILE* file);
    BIN_class* readNode(FILE* file);

  void deleteRec(BIN_class* node);

  BIN_class* root;
  int depth;
  
    static int size;

    static int checks;

  static Graph** MODELS_IN_TREE;
};


#endif
