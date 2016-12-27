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

#ifndef _token
#define _token

#include <stdio.h>
//#include <stream.h>
#include <iostream>
#include "assert.h"

#ifndef MAX_MODEL_SIZE
#define MAX_MODEL_SIZE 500
#endif

using namespace std;

class Token{

 public:

  Token(int D);
  Token();
  ~Token();
  Token(Token& tok);

  void init(int D);



  void set(int ind,int el);
  void set(int* eptr);
  
  void setErr(int ind,double e);
  void setErr(double* dptr);
  void setEdgeErr(double e);
  void addEdgeErr(double e);

  int length();

  Token operator=(Token &tok);
  int operator[](int ind);
  int operator<(Token  &t);
  Token*  operator+(Token &t);
  Token* operator*(Token &t);

  int intersect(Token* t);
  double calcError();

  double err(int ind);
  double totalErr();
  double edgeErr();

  void dump();

  void write(FILE* file);
  void read(FILE* file);


  int* t;
  double* error;
  double tot_err;
  double edge_err;

  Token* inparents[2];
  int dim;
 

  static int inst_count;
  static int sumOfLength;
  static double maxError;
  static int StatLevel[MAX_MODEL_SIZE];
 
};



#endif
