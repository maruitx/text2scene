/********************************************************************
              Copyright 1995 University of Bern              
     
                   All Rights Reserved
 
Permission to use this software and its documentation for any purpose
 and without fee is hereby granted, provided that the above copyright 
notice appear in all copies and that both that copyright notice and this 
permission notice appear in supporting documentation. Do not distribute 
without specific, written prior permission.  
                                                  
Author: Bruno T. Messmer	                       
Email:  messmer@iam.unibe.ch	                   
********************************************************************/

#ifndef _KB
#define _KB
#include <stdio.h>
#include <signal.h>
#include "Graph.h"
//#include "AttributeClass.h"
#include "AttControl.h"
#include "AdjazenzMatrix.h"
#include "Tools.h"
#include "ComNet.h"
#include "Matcher.h"
#include "Decision.h"

#define MAX_MODELS 800
#define SUBGRAPH 100

#define TREE 1
#define SECR 2
#define POLY 3

extern int STOP_BIT;




void answer_signal(int signum);

/********************************************/
/* Knowledge Base Class                     */
/* Use only methods in the nice sector      */
/********************************************/


class KB{


 public:

  KB();
  ~KB();

/*********************************/
/********* NICE INTERFACE ********/
/*********************************/

  void initFromFile(char* name);
  void setMethod(int method, int variante, double pars);
 
  
  int loadDatabase(char* name);
  int saveDatabase(char* name);

  void startNewProject(char* name);
  void loadProject(char* name);
  void saveProject(char* name);

  /* add model graph and compile immediately into network */
  void addModel(Graph *G,int key);

  /* add Model and update with already stored instances  */
  void addModelAndUpdate(Graph *G,int key);

  /* add model by means of creating a MetaGRaph */
  void addGraphs(List& g,List& k,int type);
    
  /* get models by key   */
  Graph* getModel(int key);

  /* get models by index */
  Graph* operator[](int i);

  int numberOfModels();

  /* set the retrieval key of model */
  void setKeyOfModel(int x,int key);

  /*interpret the input graph I */
  void recognition(Graph *I);
  void recognition(Graph *I,double threshold);
  Graph* recognition(char* name);
  Graph* recognition(char* name,double threshold);

  /* continue interpretation */
  void continueRecognition();
  void continueRecognition(double threshold);


  /* remove specific instance and partial instance out of network */

  void removeInstances(InstanceData* IS);

  /* remove all found instances */
  void clear();

  /* remove all graphs and delete the network */
  void discard();
  
  /* ask for found instances */  
  int NumberOfMatches();
  int NumberOfTotalMatches();
  InstanceData* getNewInstance(int i);
  InstanceData* getInstance(int i);

  /* ASCII output of interpretation to stdout */
  void displayInterpretation();
  void displayAllInterpretation();

  void displayModelBase();
  
  /* if application owns graphs call ownsGraph(0)       */
  /* otherwise, with ownsGraph(1), graphs will be freed */
  /* if no longer used                                  */

  void ownsGraph(int i);

/****************************************/
/*****END NICE INTERFACE*****************/
/****************************************/

 
  AttributeClass* getAttributeObject();
 
 
  //void setTimerForEstimation(Timer* Tx,Series_Type* Et); 

 
  void addModel(char* name);
 
  int  status();
  void ErrorMsg(const char* text,int pr);
  int ConditionsTrue();
  void dump(MappingData* MD);
  void dump(InstanceData* ID,char* file);

  void getDefaultParameters(int*& parms);
  void getParameters(int* &parms);
  void setParameters(int* parms);
  void readParameterFile(char* name,int* p);
  void interpretParameterfile(FILE* file,int* p);

  void setMatchingMethod(int m);
  void prepareFromBase(int* pset);


  double totalError(AdjazenzMatrix *O,AdjazenzMatrix *I,MappingData *MD);

  void getStatistics(int* expansion,int* memory,long* time,int* instances,double* minError, int* checks);

  void getNetworkStructure(int* iarray,int& n);
  List* getNetworkData();

  int* distributionPartials(int& count);
  int initQuery(int so);
  MappingData* getPartialMatchings(int *n,double *size);
 
  int admissibility(Graph *G);

  void setImage(Graph* G);

  void setUpperThreshold(double ut);

  char* getAttributeFilename();

/* data structures */

  //AttributeClass ATT;

  NetWork N;

  Matcher T;

  PG_class P;

 private:
  
  char DEFINITION_NAME[256];

  char SETTING_FILE[128];
  char SETTING_FILE_INFORETRIEVAL[128];

  int p[7];
  int p_retrieval[7];

  int Default_MODES[7];

  int mode;
  int match;
  int Definition_flag;
  int Modelgraph_flag;
  int internal_status;

  int graphs_owned_locally;

  ModelDataBaseType ModelData[MAX_MODELS];

  int modelNumber;
  int MatchingMethod;
  int MatchingVariante;
  double MatchingPars;
  
  Graph* Image;


  List SUB_list;
  List DEL_list;
  List INS_list;


};

#endif
