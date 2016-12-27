#ifndef mappingdata_h
#define mappingdata_h

#include <stdio.h>
#include <string.h>
#include "Token.h"
//#include "AttributeClass.h"
#include "AttControl.h"
#include "AdjazenzMatrix.h"

#define MIN_LEN 20

class MappingData{

  public:
  MappingData();
  MappingData(int dim);
  ~MappingData();
  void isMapped(int* x,int* y, int i);
  int count();
  void map(int x,int y);
  void setName(char* name);
  char* Name();
  void setNumber(int num);
  int getNumber();

  void setMatch(Token *tok1,Token *original);

  int lookup(int x);
  int lookup2(int x);

  void dump();

  Token* Original();
  Token* Image();
  int originalNodeTo(int n);


  private:

  void realloc_me();
 
  struct mapped{
    int n1;
    int n2;
  };

  
  char name[256];
  int number;

  mapped* ONTO;
  int length;
  int Count;
  
  Token *original;
  Token *model;


};



#endif






















