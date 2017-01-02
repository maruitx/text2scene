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

#ifndef _multi
#define _multi

#include <string.h>

#include "gmt_assert.h"
#include <stdio.h>
//#include <stream.h>
#include <iostream>
#include "List.h"
#include "Tools.h"

/************************************************/
/*  MultiIndex:  takes void* pointer as data    */
/*                                              */
/************************************************/

using namespace std;


class MultiIndex{


public:
  
  MultiIndex();
  ~MultiIndex();
 
  void init(int d);

  void insert(void* E,INT_TYPE* keys,int d);
  
  void* get(INT_TYPE* index, int d);
  int count();

  void clear();
  
private:

 SortedList* Root;
 int depth;

 void recursiveClear(SortedList *S,int d);

};

 

#endif
