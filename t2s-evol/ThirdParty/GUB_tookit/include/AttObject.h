#ifndef _attobject
#define _attobject
#include <stdio.h>
#include <stddef.h>


struct ComplexDataType{
  void* data;
  int descriptor;
};

class AttId
{
public:

  AttId();
  AttId(int l,double* v,int d);
  AttId(AttId &at);
  ~AttId();
  void set(int l,double* v,int d);

  void setComplex(void* data,int descriptor);

  AttId operator=(AttId& at);
   AttId operator<(AttId& at);
  AttId operator=(int id);
  int operator==(AttId& at);
  int operator!=(AttId&at);
  int operator==(int id);
  int operator!=(int id);
  
  void write(FILE* file);
  void read(FILE* file);

  double operator[](int x);
  int label();
  int length();
  double* Values();
  void* ComplexValue();


/* data structures */
  
  ComplexDataType* complex_value;
  double* values;
  int n;
  int Label;
  int ForeignMemory;
};
  

extern AttId NO_ATTRIBUTE;


#endif

