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


#ifndef _PG_class
#define _PG_class

#include <stdio.h>
#include "List.h"
#include "Graph.h"
#include "BRANCH_class.h"
//#include "TimerSO.h"
#include "MappingData.h"
#include "Token.h"
#include "AdjazenzMatrix.h"
#include "Multiindex.h"

extern AttributeClass ATT_object;


const int MAX_MODELS=800;
const int  MEM_LIMIT=1;


extern int default_valid;
extern int GLOBAL_TAG;
extern int ENV_COMPLETE;
extern int ENV_MIN_GRAPH_LENGTH;
extern int ENV_BUILD_LEVEL;
extern int ENV_CONSISTENCY_TEST;
extern int ENV_PRUNING_TREE;
extern int ENV_PRUNE_INSTANCE_LIMIT;
extern int ENV_PRUNED_MAX_COUNT;
extern int ENV_COPY;
extern int ENV_DEBUG;
extern int ENV_NOSAVEINSTANCES;
extern int ENV_NOSAVEINSTANCES_ATSAVETIME;
extern int ENV_ACCEPT_FIRST;
extern int ENV_MAX_GRAPH_LENGTH;
extern int ENV_DECISION_TREE_LIMIT;
extern int ENV_FAST_BUILD;
extern int ENV_BYTREE_DIFF;
extern int ENV_ONLY_TRUE_STATES;
extern int ENV_SEARCHING_LINEAR;
extern int ENV_NOREDIRECT;
extern int ENV_NOFASTSEARCH;
extern int return_last_always;



struct RC_type{
    int column;
    int row;
};

struct DT_node{
  List sequences;
  INT_TYPE depth;
  BRANCH_class branch;
  DT_node** successors;
  int succ;
  int allocated;
  INT_TYPE* permutation;
  int* models;
  int  models_count;
  int* inst_count;
  int mark;
  INT_TYPE terminal;
  List sequences_indicator;
    int tag;
};

struct Indicator_type{
    DT_node* node;
};


struct Type_part{
    int seg;
    int* seq;
};

struct Type_largePart{
    int seg;
    int* original;
    int* sequence;
    int length;
    int model;
    DT_node* state;
};



struct SEQ_type{
  int vertex;
  SEQ_type* up;
};

extern SEQ_type DUMMY_SEQ;


class PG_class{
    
public:
    
    PG_class();
    ~PG_class();
    
    void setMethod(int var, double pars);
    int admissibility(Graph* G);

    int recognition(Graph *G);

    void addModel(Graph *G, int num);

    int merge1(Graph *G,int num);

    int merge2(Graph *G,int num);

    void mergeWithDistortions(Graph* G,int num);

    void auxilaryModels(List& l);

    int mergeLargeGraph(Graph* G,int num);
    int recSegments(INT_TYPE** segment,int* AVAIL,int* seg_length,int cur);
    
    int buildNextLevel(Graph *G,int num,int level);

    int isomorph1(Graph *G);

    int isomorph2(Graph *G);

    int prunedIsomorphism1(Graph *G);

    int prunedIsomorphism2(Graph *G);

    int largeIsomorphism(Graph* G);
  
    int largeIso_rec(Graph* G,Hash& USED,List& Partials,Hash& done_segments);

    int subisomorph(Graph *G);

    int subisomorph_func(Graph *G,Hash& USED,int starting_vertex);

    int NumberOfMatches();
    MappingData* query(int c);


    void clear();
    void discard();
    int dump(char* fname);
    int dump();
    int dumpResults();
    int dumpTree();
    int recursiveDescent(DT_node* state,int level,int limit);

    int numberOfModels();
    Graph* getModel(int i);

    DT_node* run1(DT_node* state,int step,int level);
    DT_node* run2(DT_node* state,int step,int level);


    DT_node* buildfromSubsets(Graph* G,int num, int level);

    DT_node* buildfromSubsets_bytree1(Graph* G,int num, int level);
    DT_node* buildfromSubsets_bytree2(Graph* G,int num, int level);

    DT_node* searchLinearInStates(INT_TYPE* seq,int d);

    int collectPermutations(DT_node* state, int level, List& Perms, List &IND, int num); 
	
    int redirectNode(DT_node* state,int* edges_tuple,INT_TYPE* permutation,int d,DT_node* nstate,INT_TYPE* first_perm, int num,INT_TYPE vertex);

    int addNode_bypass(DT_node* state,int* edges_tuple,INT_TYPE* permutation,int d,DT_node* nstate,INT_TYPE* first_perm, int num,INT_TYPE vertex);

    int addNode(DT_node* state,int* edges_tuple,int num,INT_TYPE vertex);

    int initSequences(int d,int fix);
    int nextSequences(INT_TYPE* sv);
    SEQ_type* backup(int &level,INT_TYPE* vs,SEQ_type* Seq);
    SEQ_type* forward(int &level,INT_TYPE* vs,SEQ_type* Seq);
 
    int nonValidSequence();

    void initSubset(int d,int l);
    int nextSubset();

    DT_node* makeOneStep(DT_node* state,int step);
    DT_node* makeOneStepInMerge(DT_node* state,int step,INT_TYPE* permutation);

    void setNodeData(DT_node* state,int num);

    void collectMatches(INT_TYPE* vertex_sequence,DT_node* state);
    void calculateError(Token *tok1,Token* tok2,AdjazenzMatrix *AM1,AdjazenzMatrix *AM2);

    void addInstance(DT_node* state,INT_TYPE *seq);
    int consistencyTest(Graph* G);
    int recursiveCollectStates(List& STATES,DT_node* state,int l,int level);

    void collectPartialMatches(INT_TYPE* vertex_sequence,DT_node* state);
    
    void collectPrunedInstances(int* start_pos,int* end_pos,DT_node** the_state,int parts,INT_TYPE* vertex_sequence);

    void deleteRecursively(DT_node* state);

    
    void positionStates(Hash& STATES,int*& Rows, int& Depth);
    void enumerateAndLocateStates(DT_node* state,int &m,int* Rows,Hash& STATES);

    void enumerateStates(DT_node* state,int &m);
    
    void drawConnections(void (*func(int,int)),int all);
    void enumerateAndDrawStates(DT_node* state,void (*func(int,int)),int all );
    

    void setModelDataBase(ModelDataBaseType* MB,int n);
    int write(char* name);
    int writeState(DT_node* state,FILE* file);
    int read(char* name);
    DT_node* readState(FILE* file);
    void setDrawFunc(void (*func(int,int)));

    int orderInLine(INT_TYPE* vs,int start,int level,int coherent_length);

    void getStatistics(double& time,int& size,int& checksi);
    void getStatistics(int *expansion,int* memory,long* time,int* instances,double* minError, int* checks);

    void searchNecessaryStates(List& States, DT_node* state);

    int confirmValidStates();
    int recursiveConfirm(DT_node* state,int level,int limit);

    void tag_op(DT_node* state,int i);

    int recognitionWithDistortions(Graph* G);


    int checks;

    int statesWritten;
    int models_checked;
    int saveNamesOfModels;

 private:
  
  // Decision Tree root node
    DT_node* RootNode;
  
  // List of graphs represented in decision tree
    List G;
  int numberOfGraphs;
  int Dim;

    AdjazenzMatrix *AM;
    AdjazenzMatrix *OriginalAM;

  /** Sequence Structure        **/
  SEQ_type* Seq;
  
  
    int* vertex_checked;
    int vertex_checked_dim;
    int vertex_checked_fix;
  int level,back;
  int* AVAIL;
 
 /** end of sequence structres  **/

    INT_TYPE* vertex_sequence;
    int* edges_tuple;
    INT_TYPE** segment;

  int step;
  

    int* subset;
    int subset_lim;
    int subset_dim;

  // merging 1 or matching 0
 

    SortedList Collection;
    int MODE;

    int largeGraphMerge;
    int seg_current;

    int minSizedModel;
    int maxSizedModel;
    
   
    
    int numberOfNodes;
    int numberOfSeq;
    double time_spent;

    int BUILDING_TREE;
    int* H_seq;

    DT_node* Result_State;

    int PASSED_BY_NODE;

    Graph* Models_in_tree[MAX_MODELS];

    Hash REF;
    SortedList Leftside;
    int NumberOfModels;
    
    List PartialMatches;

    int current_start_pos;

    int debug;
    int valid_states;

    void* (*draw_func)(int,int);
   
    int testing_consistency;

    List delInstances1,delInstances2;

    MultiIndex *HashSeq1,*HashSeq2;
    

    int VARIANTE;
    double PARAMETERS;

    List AuxilaryModels;
    Hash OriginalModels;

};


#endif
