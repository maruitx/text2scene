#include <stdio.h>
#include "MappingData.h"


MappingData::MappingData(){
  //ONTO = new mapped[MIN_LEN];
  original=NULL;
  model=NULL;
  length = MIN_LEN;
  Count = -1;
}


MappingData::MappingData(int dim){
  //ONTO = new mapped[dim];
  length = dim;
  Count = -1;
}


MappingData::~MappingData(){
  //delete ONTO;
  if(original)
    delete original;
  if(model)
    delete model;
  original=NULL;
  model=NULL;
}


void 
MappingData::setName(char* n){
  strcpy(name,n);
}

char*
MappingData::Name(){
  return name;
}

void
MappingData::setNumber(int n){

  number=n;
}

int 
MappingData::getNumber(){
  return number;
}


void
MappingData::isMapped(int* x,int *y,int i){

  if(i<Count){
    *x=ONTO[i].n1;
    *y=ONTO[i].n2;
  }else{
    printf("NO such index");
  }
}



void
MappingData::setMatch(Token *tok1,Token *ori){
  
  original=new Token;
  model=new Token;
  (*original)=(*ori);
  (*model)=(*tok1);

}


int 
MappingData::count(){

  return Count+1;
}






void
MappingData::map(int x,int y){
  int i;

  i=lookup(x);

  if(i>-1) printf("Overwritten matching");

  Count++;
  if(Count>=length) realloc_me();

  ONTO[Count].n1 = x;
  ONTO[Count].n2 = y;
 
}



int
MappingData::lookup(int x){

  int i;

  for(i=0;i<Count;i++)
    if(ONTO[i].n1==x) return ONTO[i].n2;

  return -1;
}


int 
MappingData::lookup2(int x){

  int i;

  for(i=0;i<Count;i++)
    if(ONTO[i].n2==x) return ONTO[i].n1;

  return -1;
}


void
MappingData::realloc_me(){
  int x;
  mapped* h;
  
  h=ONTO;
  ONTO = new mapped[2*length];
  for(x=0;x<length;x++)
    ONTO[x]=h[x];

  length = 2*length;

  delete h;

}


void 
MappingData::dump(){
  int y;
  
 
  cout << "ModelNr:" << number << " : \n"; 
  cout << " ModelIndex - ImageIndex     \n";

  for(y=0;y<original->length();y++){
    cout << ""; 
    cout << (*original)[y];
    cout << "-";
     cout << (*model)[y];
    cout << " \n ";

  }
  if(model->totalErr()>0) cout << " Err:" << model->totalErr();
  
}





Token* 
MappingData::Original(){
  return original;
}

Token*
MappingData::Image(){
  return model;
}


int
MappingData::originalNodeTo(int n){
  int x,i;
  
  i=-1;
  for(x=0;x<original->length();x++){
    if((*original)[x]==n){
      i=x;
      break;
    }
  }

  gmt_assert(i!=-1);

  return (*model)[i];
}
