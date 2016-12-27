#ifndef _tools
#define _tools

//#include <stream.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
//#include <cmath>
#include "time.h"
#include "List.h"

using namespace std;

#ifndef INT_TYPE
#define INT_TYPE char
#endif

#ifdef SOLARIS
#define PI M_PI
#else
#define PI 3.1417
#endif

#ifdef GCC
#define PI 3.1417
#endif

extern const double FPepsilon;

char getc_c(FILE* file);
double get_zahl(FILE* file);

int checkMatch();

void initRand();
//long elapsed_time();
time_t elapsed_time();

void Clique(int i,int* Best,int* Current,int& Besti,int& Currenti,int Lim,int** mx);


struct Edit_Desc{
  int edge_o;
  int edge_i;
  double err;
};


double HS(double** m, int dim_l,int dim_r);

double HS_smart(double** m, int dim_l,int dim_r,int* path);

void collect_edge_edits(int dim_l,int dim_r,int* path,int* edge_l,int* edge_r,double  **m,List& SUB_list,List& DEL_list,List& INS_list);


struct Series_Type{
  Series_Type* next;
  double time;
  double err;
};




void truncateName(char* name);

class AdjazenzMatrix;



int depthFirstTreeSearch(AdjazenzMatrix* Model,AdjazenzMatrix *Image,INT_TYPE* vertices,int* images,int start_level,List& Instances);


#endif
