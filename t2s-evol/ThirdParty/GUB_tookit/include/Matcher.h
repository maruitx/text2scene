#ifndef _matcher
#define _matcher

#include <stdio.h>
//#include <stream.h>
#include <iostream>
#include "AdjazenzMatrix.h"
#include "MappingData.h"
//#include "AttributeClass.h"
#include "AttControl.h"
#include "Token.h"
#include "List.h"
#include "Graph.h"
#include "gmt_assert.h"
#include "Tools.h"


/*************************************************/
/* MODES[0,1,2]                                  */
/* 0: 0 Tree 1 RETE                              */
/* 1: 1 A*   2 Ullman                            */
/* 2: 1 Error Correcting 2 Exact (only for A*)   */
/* 3: 1 monomoprhism 2 subgraph iso (only for NA */
/* 4: 1 Error correcting 2 exact (only INA)      */
/* 5: 1 no lookahead 2 static 3 dynamic          */
/*************************************************/

using namespace std;


#define MAX_MODELS_TREE 610

#ifndef MAX_MODEL_SIZE
#define MAX_MODEL_SIZE 610
#endif

struct TNode{
  
  double fferror;
  int original;
  int image;
  int level;
  TNode* parent;
  
};



class Matcher{


 public:

  Matcher();

  void clear();

  void initAttributes(AttributeClass *A);

  void setModelDataBase(ModelDataBaseType* MD,int count);

  void match(AdjazenzMatrix* AM);

  void match(AdjazenzMatrix* AM,double u_thres);

  void matchContinue(double u_thres);

  int NumberOfMatches();

  MappingData* query(int c);

  

  void treeSearch();

  int verify(TNode *leaf);

  Token* followUp(TNode *Next);

  int isAvail(int avail, int x);

  double edgeError(int node,int o_node,TNode* Next);

  double subIsoEdgeError(int node,int o_node,TNode* Next);

  double isomorphismError(TNode* Son);

  void collectMatch(TNode* Next);

  void getStatistics(int *expansion,int* memory,long* time,int* instances,double* minError,int* checks);

  void statistic(char* name);

  int* distributionPartials(int &count);

  int time();

  void deleteTrees();

  void deleteSearchTree();

  void marchBackwards(TNode* NX,TNode** delarray,int &x);

/** ULLMAN **/
  void setParameters(int *p);

  void collectUllmanMatch(int* H);

  void UllmansAlgo();
  void Ullman(int d,AdjazenzMatrix* MO,int*** FET,int n,int m);
  int refine(int feti,int d,int c,int n,int m,int*** FET,AdjazenzMatrix* MO);

/** Ullman for InfoRetrieval */
  
  void UllmansAlgo(AdjazenzMatrix *AM, Token* common, AdjazenzMatrix* COMMON, List* Unique, List* Ref);
  void Ullman(int flag,int ind, int d, unsigned char**** FET, unsigned char*** CFET, Token* common, AdjazenzMatrix* COMMON, List* Unique, List* Ref, int* ACTIVE);
  int refine(int flag,int model,int feti,int d,int c, unsigned char*** CFET,unsigned char**** FET,AdjazenzMatrix * COMMON,Token* common, List* Unique,List* Ref,int* ACTIVE);
  
  void collectUllmanMatch(int flag,int* AVAIL_2,int x,AdjazenzMatrix* COMMON,Token* common,List* Unique,List *Ref);
  
  int expandedNodes,sumSteps,check_count;

  int StatLevel[MAX_MODEL_SIZE];
  int StatLevelCount;
 
  double cut_threshold;
 
  SortedList Collection;
  SortedList TotalCollection;

  int RETURN_ON_FIND;
 
private:

  int MODES[7];

  SortedList *OPEN;
  SortedList *OPENTREES;
  List *TERMINALS;
  SortedList BESTTREE;
  List* Terminal;
  
  int *AVAIL,*SLEEP;

  long startTime,endTime;
  int DeepestLevel;
  double maxError;
  
  ModelDataBaseType* MB;
  AdjazenzMatrix* AM;
  AttributeClass *ATT;

  int numOfModels;
  int CurrentModel;
  AdjazenzMatrix* CurrentMatrix;

 

  Token* OriginalModels[MAX_MODELS_TREE];
  Token* Original;
  
  TNode* Start;

  double THRESHOLD;
  


  

};


#endif
