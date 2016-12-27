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


#ifndef _comnet
#define _comnet
#include <stdio.h>

#include "MappingData.h"
#include "AdjazenzMatrix.h"
//#include "AttributeClass.h"
#include "AttControl.h"

#include "Graph.h"
#include "MetaGraph.h"
#include "List.h"
#include "Token.h"
#include "assert.h"


class In_MetaGraph;
class MetaGraph;

//#ifdef SOLARIS
//#include "TimerSO.h"
//#else
//#include "TimerU.h"
//#endif

#include "Tools.h"



#define RUN_MODE 1
#define COMPILE_MODE 2
#define DEBUG_MODE 3
#define TIME_MODE 4
#define STEP_MODE 5
#define TIMESTEP_MODE 6
#define NORMAL 1
#define BALANCED 2
#define NET 1        // BALANCED, NORMAL,
#define NETMATCH 2   // MONO,SUBGRAPH_NODES, PRUNED
#define MATCH 3      // MONO,SUBGRAPH1, SUBGRAPH2 ,ISOMORPHISM
#define ALGO 4       // SECR,LECR
#define SCENE 5      // SINGLE,SCENE
#define TREAT 10
#define CONTINUE 20
#define SAVE 30
#define EXIT 40
#define LEFT 0
#define RIGHT 1
#define NNODE_TYPE 1
#define ENODE_TYPE 2
#define CNODE_TYPE 3
#define MNODE_TYPE 4


extern double HEURISTIC1;

const double MAX_COST=300000.0;

/**** internal types  ***/

struct BridgeData{
  int n1;
  int n2;
  AttId id;
  int direction;
};


struct ModelDataType{
  int Number;
  char name[64];
  char description[256];
  AdjazenzMatrix* AM;
};


struct ConnectType{
    int NType;
    int ind;
    int direction;
};



struct action_element{
  int node;
  int type;
  Token* tok;
  Token* etok;
  int index;
  int direction;
};
    

struct update_element{
  List left;
  List right;
  int node;
  int type;
  List* tmp_go;
};


struct GeneralInfoStruct{
  int UID;
  int key;
  List vertices;
  int type;
  int node;
  List successors; // UIDs
  void* data;
};


class NNodeType{  
 public:

    List GO;
    SortedList STOP;
    SortedList STOP2; // used for combined lookahead

    List TMP_GO;

    double FUTerr_average;
    List top_instances;
    int represented;

    AttId TestID;
    List testList; 
    int totalEdges;
    int TopNumber;
    List left_successor,right_successor;
    ConnectType parents[2];
    List models;

    SortedList FUTlist;
    List EdgeGO;

    int UID;

    double FUT;
    double FUTerr;
    double assignedFUTerr;
    Token* FutErrorToken;
    double totalDeletionCost;

#ifdef STUDIES
    double timeSpent;
    int testTry;
    int testPassed;
#endif

    int newLength;
    int terminal;

    NNodeType();
    ~NNodeType();
    NNodeType(NNodeType &n);
    NNodeType operator=(NNodeType &n);

    void clear();
    void shallowClear();
    void discard();
    void write(FILE *file);
    void read(FILE *file);
    void dump();
    void dump(FILE* file);

};


class CNodeType: public NNodeType{

 public:


  List SGI;
  
 
  CNodeType();
  ~CNodeType();
  void discard();
  CNodeType(CNodeType &n);
  CNodeType operator=(CNodeType &n);
  void write(FILE* file);
  void read(FILE* file);

  int testSteps;
    
};


class MNodeType{

 public:  
  List parents;
  int modelNr;
  char* modelName;
  Token Original;
  int TopNumber;
  int UID;

  List instances;

  // is used only in the case of isomorphism
  SortedList STOP;
  
  MNodeType();
  ~MNodeType();
  MNodeType(MNodeType &n);
  MNodeType operator=(MNodeType &n);
  
  void write(FILE* file);
  void read(FILE* file);

  void clear();
  void discard();

};
  

class NetInfoType{

 public:

    List originalNodes;
    List originalEdges;
    List originalModels;

    NetInfoType();
    ~NetInfoType();

    NetInfoType(NetInfoType &n);
    NetInfoType operator=(NetInfoType &n);
    void dump(FILE* file);
    void write(FILE *file);
    void read(FILE *file);

    void clear();
    void discard();

    int getOriginals(int m,Token** tok,Token** etok);

};




/************************************************/
/*     NetWork Class                            */
/************************************************/


class NetWork{


 public:

  NetWork();
  ~NetWork();


  void initAttributes(AttributeClass *ATT);

  int read(char* name);
  int write(char* name);

  void setData(AdjazenzMatrix *M);
  
  //void setTimerForEstimation(Timer *Tx,Series_Type *Et);

  void lateCompile(AdjazenzMatrix *M,int num);

  void compile(AdjazenzMatrix *M,int num);

  void compileBalanced(AdjazenzMatrix *M,int num);

  void compileForSGI(AdjazenzMatrix *M,int num);

  void match(AdjazenzMatrix *M);

  int runSpecified(Token* tok, AdjazenzMatrix *M);

  int SECRrun(AdjazenzMatrix *M,double threshold);

  int SECRcontinue(double threshold);

  void run(int node);

  void REMOVErun(int* vertices,int n);

  void topologicalSort();

  void clearNet();

  void discard();
  
  int NumberOfNewMatches();
  MappingData* queryNew(int c);


  int initQuery(int argument);
  int getPartialMatchings(List* MD_list, double *size);

  int NumberOfMatches();
  MappingData* query(int c);
  
  void enclosingNodes();
  
  void setMode(int m);

  void setGrainInNetwork();

  void debug();

  void debug(int node,int type,Token* arg,int state);

  void  display(int ans,List* GO,SortedList* STOP, List* orig, Token* arg,Token* ltest,Token* rtest);

  void statistic(char* name);
  void getStatistics(int *expansion,int* memory,long* time,int* instances,double* minError, int* checks);

  int* distributionPartials(int &count);

  void setDebug();
  
  void debugDump();

  void continueProcess(AdjazenzMatrix* M);

  void process();

  void dump(FILE* file);  

  void setModelDataBase(ModelDataBaseType* MDB,int num);

  void lockModel(int i);

  double subgraphError(Token* tok,int i);

  double isomorphismError(Token* tok);

  void setParameters(int *p);

  void registerCallback(void* fptr); /** NetWork given as a parameter **/


  int time();

  void displayTreeAscii();


  /**** test function *****/



  int getTokenPair(action_element* P,Token** tok1,Token** tok2);

  Token* NTest(int i,Token* tok);

  Token* CTest(int i,Token* tok1,Token* tok2);
 
  void SGItest(Token* tok,int i);

  
  void enterCurrentBestTry(int i,Token *tok);


  void getNodeInfo(int type,int ind,int &open_nr,int& closed_nr,double& FE,double& minimal,double& nextbest,double& second_heuristic);

  void getAncestors(int type,int node,Hash& ht);
  List* getNetworkData();


/** flexible compilation from MetaGraph **/

  void buildFromMetaGraph(List *G,int ModelStartNumber);
  MetaGraph* buildNetwork(In_MetaGraph *G);
  ConnectType* createSubgraphMatcher(In_MetaGraph* G,In_MetaGraph *G1,In_MetaGraph* G2);
  ConnectType* createVertexChecker(In_MetaGraph* G);

/****************************************/

  double individualLookahead(Token* tok, int node,int type);
  int renewEstimation(action_element* P);

  int StatLevelCount;

  // special function which allows user defined identity test, such that
  // vertices which may be be multiply matched, are not rejected

  int (*disjoint_test_func)(int x,int y, AdjazenzMatrix* A);

// private:

  void  (*callbackPtr)(NetWork* N);

  int MODES[7];

  int REMOVE_MODE;

  int BREAKPOINT_SET;
  ConnectType BREAKPOINT;
  int STOPPEDBYUSER;

  int SUBGRAPH_ISO;
  int GRAIN;
  int DEBUGSET;
  int POLLINGSET;

  double cut_threshold;
  //extern double HEURISTIC1;

  NNodeType *NNodes;
  NetInfoType *InfoNN;
  int NNodeDim,NNodeX;

 
  CNodeType *CNodes;
  NetInfoType *InfoCN;
  int CNodeDim,CNodeX;
  MNodeType *MNodes;
  NetInfoType *InfoMN;
  int MNodeDim,MNodeX;

  int mode;
  int UIDLimit;   /* unique identity number limit */


  int ERROR_ESTIMATION;
  int UPDATE_ESTIMATION;
  int UPDATE_UID;
  int WORST_ESTIMATION;

  SortedList CurrentBestTry;


  SortedList CompileBest;
  ModelDataBaseType* ModelData;
  int ModelDataNumber;

  AttributeClass *ATTC;
  int attributeInit;

  AdjazenzMatrix *WModel;
  int WMNumber;
  int WMDim;

  int* VX,*EX;

  Stack ACTION;
  SortedList OPEN;

  SortedList Collection,TotalCollection; 

  SortedList QueryList;

  long startTime,endTime;
  int ctest_count;

  action_element* setActionElement(int node, int type, void* tok, int index);
  void ruleMove(int node,int type,Token* tok,Token* etok,List* left,List* right,List *models);
  void reallocNodes();
  void reallocNodes(int v,int e,int modelnum);


  void saveIn(action_element* P);
  void treatIn(action_element* P);

  void insertIntoOPEN(int node,int type,int  uid,Token* tok);
  void insertIntoUPDATE(int node,int type,int uid,List* tmp_go,ConnectType* CT,int dir);


  /*** Compilation supporting cast  **/
 
  action_element* findPartial(Token* tok,ConnectType* C,List* Elist);
  int findEntry(Token *tok,List* go,Token **rtok,List* Elist);
  action_element* createNode(action_element* P,Token* tok,List* Elist);

  void findBridges(Token* tok1,Token* tok2,List* Elist);

  int isComplete(Token* etok);

  void TetMarch(int node);

  void moveTokenToINFO(ConnectType *C,Token *etok1);

  void removeFromGO(int node,Token *etok);
  int removeFromGO(Token* tok,List& GO,double &FUTerr,double &FUT);
  int removeFromSTOP(Token* tok,SortedList& STOP);

  void orderSuccessors();

  void consistencyCheck();

  void polling();

 /** Balanced supporting cast  **/
  
  SortedList CompileBalanced;
  int ACT_UID,ACT_UID_P;
  int COMPILER_MODUS;


  int isUsedInBalanced(Token* tok,int auid);
  int  setPartBalancedList(ConnectType *C);
  void setBalancedList();
  void markTokenForBalanced(int type,int node);
  void findEdgeClique(List* edgego,int uid,int key,ConnectType* C);
  


  void determineNoEdgeTests();
  int edgesAreEnclosed(int v,Token* etok,int mod);
  void fillNoEdgeList(Token* rtok,int i);

  void LECRFutureError(int t);
  void LECRresortOPEN();
  Token* LECRErrorToken(double fut_l,double fut_r,double err_l,double err_r,Token* tok_l,Token* tok_r,int node,int type);
  void LECRassignFUTerr();
  void LECRassignFUTrec(ConnectType *C,double err);
  void LECRstartDynamicEstimate(action_element* P);
  void LECRupdateFUT(action_element* P,double err,ConnectType* CT);

  /* second heuristic functions */
  int averageFUTerr(NNodeType* Node,Token* tok);
  double getSecondHeuristic(ConnectType& C_from,ConnectType& C_to);


  Series_Type *Estimation_Series;
//  Timer *Timex;

  int removed_instances;
  int NetworkDepth;

  int ENV_CONTROL_STACK;
  Stack STACK;
  List *CONTROL_GO;
  
  void prepareOPENandSTACK();
  void recursiveSTACKfill(ConnectType* CT);
  void prepareOPEN();
  void recursiveOPENfill(ConnectType* CT);

  SortedList UPDATE_LIST;

  int Late_Compile;

  Hash Hidden;

  int STORE_IN_STOP;
  int RETURN_ON_FIND;
  
};


double edgeErrorBetween2Vertices(AdjazenzMatrix* WModel, AttributeClass* ATTC, AttId* e_l,int dim_l,int l,int r,int dir);

int removeFromGO(Token* tok,List& GO);
int removeFromSTOP(Token* tok,SortedList& STOP);

#endif

