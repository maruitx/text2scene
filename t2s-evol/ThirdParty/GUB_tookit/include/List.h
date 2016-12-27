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

#ifndef _list
#define _list

#include <string.h>

#include "assert.h"
#include <stdio.h>
//#include <stream.h>
#include <iostream>
#include "Token.h"


/************************************************/
/*  List:  takes void* pointer as data          */
/*                                              */
/************************************************/

using namespace std;


class List{


public:
  
  List();
  ~List();
  List(List &l);

  List operator=(List &l);

  void insert(void* E);
  void insertAtTop(void* E);
  void reset();
  void* remove(int index);
  void deleteCurrent();
  void* getnext();
  void* get(int index);
  int count();

  void mv(List &Dest);

  void dump(char* types);
  
  int currentIndex();

  void clear();
  void clearDeep();

  int statMax;
  int statTotal;


private:

struct element{
   void* EL;
   element *next;
 };

 element* Top;
 element* Last;
 element* index_ptr,*pre_index_ptr;
 int index;
 int Count;
 


};



#if 0


class SortedList{

public:
 

  SortedList();
  ~SortedList();

  void descending();

  void insert(void* E, int key, double value);


  void* removeKey(int key);
  void* removeTop(int* key);
  void* top(int *key);
  int isKey(int key);
  void initNext();
  void* getnext(int *key);
  void* get(int i);


  double TopValue();

  int count();
  void showItems();

  void dump(char* types);

  void clear();

  int statMax;
  int statTotal;

private:

struct sortelement{
   void* EL;
   double value;
   int key;
   sortelement* next;
};

  int BIGFIRST;
 sortelement* Top,*Last;
 sortelement* LIST[20];
 int ILIST[20];
 int index;
 int HCount,Count;
 sortelement* index_ptr;


  void split(int c);
  void shrink(sortelement *sptr,int c);
  int search(double value);

};
  
#else


/************************************************/
/*  SortedList:  takes void* pointer as data    */
/*               and a value and key argument   */
/************************************************/






struct sortelement{
   void* EL;
   double value;
   int key;
   sortelement* ancestor;
   sortelement* left;
   sortelement* right;
};

class SortedList{

public:
 

  SortedList();
  ~SortedList();

  SortedList(SortedList &s);

  SortedList operator=(SortedList& s);

  SortedList operator+=(SortedList& s);


  void descending();

  void insert(void* E, int key, double value);


  void* removeKey(int key);
  void* removeTop(int* key);
  void* removeElement(void* el);

  void* top(int *key);
  int isKey(int key);
  void* getKeyData(int key);
  double getKey(int key);
  void initNext();
  void deleteCurrent();
  void* getnext(int *key,double *v);
  void* getnext(int *key);
  void* get(int i,int& key, double& value);
  void* get(int i,int& key);
  void* get(int i);
  void* getValue(double v);
  int getValueKey(double v);

  sortelement* getRec(int i,int &c,sortelement* akt);

  double TopValue();

  int count();
  void showItems();

  void dump(char* types);

  void clear();
  void clearDeep();


  int statMax;
  int statTotal;
  int statDepth;

private:



  int BIGFIRST;
  sortelement* Top,*Last;


  int index;
  int HCount,Count;
  sortelement* index_ptr,*old_index_ptr;


  void split(int c);
  void shrink(sortelement *sptr,int c);
  sortelement* search(double value);
  sortelement* isKeyRec(int key,sortelement* ptr);
  void removePtr(sortelement *ptr);
  sortelement* isVoidRec(void* el,sortelement* ptr);


};


#endif

/**********************************************/
/*      Stack: takes void* as argument        */
/**********************************************/


class Stack{

 public:

  Stack();
  ~Stack();
  
    void push(int key,void* Tok);
    void *pop(int* key);
    void *peek(int* key);
    int count();
    
    int empty();
    

  private:

    struct stackelement{
      void* EL;
      int key;
      stackelement* next;
    };

    stackelement *Top;
    int Count;

  };


struct HashStruct{
  int key;
  void* info;
  HashStruct* next;
};


class Hash{
 public:
  Hash();
  Hash(int dim);
  ~Hash();
  

  int count();
  void* get(int i);
  void clear2();
  void clear();
  void insert(int key);
  void insert(void* data,int key);
  void* remove(int key);
  int in(int key);
  void* getKey(int key);
 
#if 0 
  void initKey(int key);
  void* getnext();
#endif

  int hashfunc(int x);
  
  HashStruct **HT;
  int Dimension;
  int entries;
};


#endif
