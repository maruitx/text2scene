//#ifndef _attributeclass
//#define _attributeclass

#pragma once

#define MAX_ATTRIBUTES 30

#include <string.h>
#include <stdlib.h>
#include "math.h"
//#include <cmath>
#include "Tools.h"
#include "AttObject.h"

#define ANGLE 1
#define WEIGHT 2
#define NAME 3
#define LOAD 4

//#define MAX_LABELS 300
#define MAX_LABELS 500

#define MAX_ATTID 100000

#define NEW_ATTOBJECT

using namespace std;

extern const int SUB_FUNCTION;
extern const int INS_OF_EDGES;
extern const int INS_OF_VERTICES;
extern const int DEL_OF_EDGES;
extern const int DEL_OF_VERTICES;
extern const int SUB_COMPLEX;
extern const int INS_OF_COMPLEX_EDGES;
extern const int INS_OF_COMPLEX_VERTICES;
extern const int DEL_OF_COMPLEX_EDGES;
extern const int DEL_OF_COMPLEX_VERTICES;
extern const int SUB_FUNCTION_COMPLEX;

extern const int DESCRIPTOR_GRAPH;

class Graph;

void registerFunction(int typ,void* (*func)());



const double INFTY_COST=300000;

class Hdouble{

 public:

  Hdouble();
  ~Hdouble();
 
  void init(int count);
  double ind(int arr,int i);
  int add(double* values);
  double* array(int ind);
  
  void write(FILE* file);
  void read(FILE* file);

  void discard();
  void resize();
  
 private:

  double ** VArray;
  int dim;
  int size;
  int index;
  
};


extern double (*substitution_cost_function)(int label1,double* values1,int n1,
					    double *w1,double *t1,
					    int label2,double* values2,int n2,
					    double *w2,double *t2,
					    double lw1,double lw2);

extern double (*vertex_insertion_cost_function)(int label1,double* values1,int n1,double ins_error);
extern double (*vertex_deletion_cost_function)(int label1,double* values1,int n1,double del_error);
extern double (*edge_insertion_cost_function)(int label1,double* values1,int n1,int f1,int f2,double ins_error,Graph* G);
extern double (*edge_deletion_cost_function)(int label1,double* values1,int n1,int f1,int f2,double del_error,Graph* G);
  

extern double (*substitution_cost_complex)(int label1,void* complex1, int descr1,
					    int label2,void* complex2, int descr2);

extern double (*vertex_insertion_cost_complex)(int label1,void* complex,int descr, double ins_error);
extern double (*vertex_deletion_cost_complex)(int label1, void* complex,int descr,double del_error);
extern double (*edge_insertion_cost_complex)(int label1, void* complex,int descr,int f1,int f2,double ins_error,Graph* G);
extern double (*edge_deletion_cost_complex)(int label1, void* complex,int descr,int f1,int f2,double del_error,Graph* G);


extern double (*substitution_cost_function_complex)(int label1,double* values1,int n1,
						    double *w1,double *t1,
						    int label2,double* values2,int n2,
						    double *w2,double *t2,
						    double lw1,double lw2,
						    void* complex1,int desc1,
						    void* complex2, int desc2);


class AttributeClass{

  public:
  
  AttributeClass(int maxLabel);
  AttributeClass();
  ~AttributeClass();
  void garbageCollection();  

  int defined();

  void init();

  void refreshParameters(int label, double* weights, double* thresholds, double INS,double DEL,double cyc);
  int rereadDefinition(char* name);
  void define(int label, double* weights, double* thresholds, int count,double INS,double DEL,double cyc);
  AttId registerLabel(int label, double* values, int count);
  void defineCostTableEntry(int label,int label1, double weight, double* rule,int dim); 
  int loadDefinition(char* name);
  int loadDefFileFromGraph(char* name);
  
  double* valueArray(AttId id,int *n);
  int Label(AttId &id);
  double error(AttId &A1, AttId &A2);
  double insertionCost(AttId &A1);
  double insertionCostOfEdge(AttId &A1,int n1,int n2,Graph* G);
  void setInsertionCost(int f);
  void setDeletionCost(int f);  
  double deletionCost(AttId &A1);
  double deletionCostOfEdge(AttId &A1,int n1,int n2,Graph* G);
  double labelToLabelError(int l1,int l2);
  
  int read(char* name);
  void write(char *name);

  void discard();

  char* name();

  int randomLabel(AttId at,double err);

  int numberOfLabels();

  int error_checks;

 private:

  int initialized;
  int REREAD;
  int lookupLabel(int l);
  char Name[256];

  int DELETION_COST;
  int INSERTION_COST;

struct AttType{

  int label;
  double cycleTop;
  Hdouble    Values;
  int dim;
  double *weights;
  double *thresholds;
  double insertionError;
  double deletionError;
};


struct  GroupType{
  int group;
  int index;
};


GroupType REF[MAX_ATTID];
AttType* LabelGroup;

int NumberOfRefs;
int NumberOfLabels;

int maxLabels;

struct  MatrixEntry {
  double weight;
  double* vweights;
};


MatrixEntry **CTable;



//static List GARBAGE;

};






//#endif









