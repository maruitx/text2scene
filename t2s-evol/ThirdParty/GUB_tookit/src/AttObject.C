#include "AttObject.h"

AttId NO_ATTRIBUTE(-30,NULL,0);

  
AttId::AttId(){
  n=0;
  values=NULL;
  ForeignMemory=0;
  complex_value=NULL;
}
  

AttId::AttId(int l,double* v,int d){
  int x;
  if(d>0){
    values=new double[d];
    for(x=0;x<d;x++) values[x]=v[x];
  }
  n=d;
  Label=l;
  ForeignMemory=0;
  complex_value=NULL;
}
 

void
AttId::set(int l,double* v,int d){
  int x;
  if(d>0){
    values=new double[d];
    for(x=0;x<d;x++) values[x]=v[x];
  }
  n=d;
  Label=l;
  ForeignMemory=0;
}
 

void
AttId::setComplex(void* data, int descriptor){
  complex_value=new ComplexDataType;
  complex_value->data=data;
  complex_value->descriptor=descriptor;
}

/****************************************************************/
/*         NOTE                                                 */
/* Copy Constructor does NOT make a true copy!                  */
/* If this causes problems, then the functionality of operator< */
/* must be set to the operstor=                                 */
/****************************************************************/

AttId::AttId(AttId& at){
  int x;
  n=0;
  values=NULL;
 
#ifdef DEEP_COPY
  
  if(at.n>0){
    values=new double[at.n];
    n=at.n;
    for(x=0;x<n;x++) values[x]=at.values[x];
  }
  ForeignMemory=0;
#else
  ForeignMemory=1;
  if(at.n>0){
    values=at.values;
    n=at.n;
  }
#endif


  Label=at.Label;
  complex_value=at.complex_value;

}


AttId::~AttId(){
  if((n>0)&&(!ForeignMemory)){
    if(values)
      delete values;
    
    //delete complex_value;
  }
}


AttId
AttId::operator=(AttId& at){
  int x;
  if(n>0) delete values;
  n=0;
  ForeignMemory=0;

  if(at.n>0){
    values=new double[at.n];
    n=at.n;
    for(x=0;x<n;x++) values[x]=at.values[x];
  }
  Label=at.Label;
  complex_value=at.complex_value;
  return *this;
}


AttId
AttId::operator<(AttId& at){
  int x;
#ifdef DEEP_COPY
// identical to operator=
 if(n>0) delete values;
  n=0;
  ForeignMemory=0;

  if(at.n>0){
    values=new double[at.n];
    n=at.n;
    for(x=0;x<n;x++) values[x]=at.values[x];
  }
  Label=at.Label;
  complex_value=at.complex_value;


#else
// only shallow copy is made!
  if((n>0)&&(!ForeignMemory)) delete values;
  n=0;

  values=NULL;
  ForeignMemory=1;
  if(at.n>0)
    values=at.values;
  n=at.n;
  complex_value=at.complex_value;
   
  Label=at.Label;
  
#endif
  return *this;
}


  
AttId
AttId::operator=(int id){
  int x;
  if(n>0) delete values;
  n=0;
  values=NULL;
  Label=id;
  return *this;
  ForeignMemory=0;
  complex_value=NULL;
}



int
AttId::operator==(int id){
  int x;  

  if(Label!=id) return 0;
  return 1;
}

int
AttId::operator==(AttId& at){
  int x;  
  if(n!=at.n) return 0;
  if(Label!=at.Label) return 0;
  if(n>0)
    for(x=0;x<n;x++)
      if(values[x]!=at.values[x]) return 0;

  return 1;
}




int
AttId::operator!=(AttId& at){
  return !(*this==at);
}

int
AttId::operator!=(int at){
  return !(*this==at);
}


double 
AttId::operator[](int x){
  if(x<n)
    return values[x];
  else
    return 0;
}


void*
AttId::ComplexValue(){
  return complex_value;
}


void
AttId::write(FILE* file){
  
  fwrite((char*)&Label,sizeof(int),1,file);
  fwrite((char*)&n,sizeof(int),1,file);
  if(n>0)
    fwrite((char*)values,sizeof(double),n,file);

}


void
AttId::read(FILE* file){
  
  fread((char*)&Label,sizeof(int),1,file);
  fread((char*)&n,sizeof(int),1,file);
  if(n>0){
    values=new double[n];
    fread((char*)values,sizeof(double),n,file);
  }
  ForeignMemory=0;
}


int 
AttId::label(){
  return Label;
}
  

int 
AttId::length(){
  return n;
}


  
double*
AttId::Values(){
  return values;
}











