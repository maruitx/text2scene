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


#include "Token.h"


int Token::inst_count = 0;
int Token::sumOfLength = 0;
double Token::maxError = 0;
int Token::StatLevel[MAX_MODEL_SIZE];



Token::Token(int Dim){
  t=NULL;
  error=NULL;
  tot_err=0;
  edge_err=0;
  inparents[0]=NULL;
  inparents[1]=NULL;
  t=new int[Dim+1];
  dim=Dim;
  
  inst_count++;
  sumOfLength+=Dim;
  StatLevel[Dim]++;
}

Token::Token(){
  t=NULL;
  error=NULL;   
  tot_err=0;
  edge_err=0;
  dim=0;
  inparents[0]=NULL;
  inparents[1]=NULL;
 
}

Token::~Token(){

  if(t) delete t;
  if(error) delete error;
  t=NULL;
  error=NULL;
  dim=-1;

}


Token::Token(Token &tok){
  int x;

  t=NULL;
  error=NULL;
  dim=0;

  tot_err=0;
  edge_err=0;

  if(tok.dim>0){
    dim=tok.dim;
    t= new int[tok.dim];
    for(x=0;x<tok.dim;x++) t[x]=tok.t[x];
    if(tok.error){
      error=new double[dim];
      for(x=0;x<tok.dim;x++) error[x]=tok.error[x];
    }
    tot_err=tok.tot_err;
    edge_err=tok.edge_err;

  }

  inst_count++;
  sumOfLength+=dim;
  StatLevel[dim]++;
  
}



int
Token::length(){

  return dim;
}


int
Token::intersect(Token *tok){
  int x,y;
  
  for(x=0;x<dim;x++){
    for(y=0;y<tok->dim;y++){
      if(t[x]==tok->t[y]) return 1;
    }
  }

  return 0;
}


Token
Token::operator=(Token &tok){
  int x;

  if(dim>0) delete t;
  if(error) delete error;
  error=NULL;
  t=NULL;

  if(tok.dim>0){
    dim=tok.dim;
    t= new int[tok.dim];
    for(x=0;x<tok.dim;x++) t[x]=tok.t[x];
    if(tok.error){
      error=new double[dim];
      for(x=0;x<tok.dim;x++) error[x]=tok.error[x];
    }
    tot_err=tok.tot_err;
    edge_err=tok.edge_err;

  }
  
  return *this;

}


void 
Token::init(int D){

  
  if(t) delete t;
  if(error) delete error;

  t=new int[D];
  dim=D;
  error=NULL;
  tot_err=0;
  edge_err=0;


  StatLevel[dim]++;
  inst_count++;
  sumOfLength+=D;
}


void
Token::set(int ind,int el){
  t[ind]=el;
}
  
void
Token::set(int* eptr){
  int x;

  gmt_assert((t)&&(dim>0));
  for(x=0;x<dim;x++) t[x]=eptr[x];
  
}
  

void
Token::setErr(int ind,double e){
  int x;

  gmt_assert(ind<dim);

  if(!error) {error=new double[dim];for(x=0;x<dim;x++) error[x]=0;};
  error[ind]=e;
  tot_err=0;
#if 0
  for(x=0;x<dim;x++) tot_err=tot_err+error[ind];
#else
  calcError();
#endif
}


void
Token::setErr(double* dptr){
  int x;
  gmt_assert(dim>0);

  if(!error) {error=new double[dim];};
  
  tot_err=0;
  for(x=0;x<dim;x++) {error[x]=dptr[x];tot_err=tot_err+error[x];}
  tot_err+=edge_err;
}


void
Token::setEdgeErr(double e){
  edge_err=e;
  calcError();
 
}


void
Token::addEdgeErr(double e){
  
  edge_err+=e;
  calcError();
}

int 
Token::operator[](int ind){

  gmt_assert(ind<dim);
  return t[ind];
}


int 
Token::operator<(Token  &tok){
  int flag,y,x;

  for(x=0;x<dim;x++){
    flag=0;
    for(y=0;y<tok.dim;y++)
      if(t[x]==tok.t[y]){
	flag=1;
	break;
      }
    if(!flag) return 0;
  }
    
  return 1;
}


Token* 
Token::operator+(Token &tok){
  int d,x;
  Token* ptr;
  
  d=dim+tok.dim;

  ptr=new Token(d);
  ptr->dim=d;

  for(x=0;x<dim;x++) ptr->t[x]=t[x];
  for(x=0;x<tok.dim;x++) ptr->t[x+dim]=tok.t[x];

  if((error)||(tok.error)){
      if(error) for(x=0;x<dim;x++) ptr->setErr(x,error[x]);
      if(tok.error) for(x=0;x<tok.dim;x++) ptr->setErr(x+dim,tok.error[x]);
    }

  ptr->edge_err=edge_err+tok.edge_err;

  ptr->calcError();

  return ptr;
}


Token* 
Token::operator*(Token &tok){
  int d,x,y,flag,i;
  Token* ptr,*hptr;

  hptr=new Token(dim+tok.dim);

  i=0;
  for(x=0;x<dim;x++){
    flag=0;
    for(y=0;y<tok.dim;y++)
      if(t[x]==tok.t[y]){
	flag=1;
	break;
      }
  
    if(flag) {
      hptr->set(i,t[x]);
      i++;
    }
  }
  
  if(i==0) return NULL;

  ptr=new Token(i);
  
  for(x=0;x<i;x++) ptr->set(x,hptr->t[x]);

  return ptr;
}


double
Token::calcError(){
  int x;
  tot_err=0;
  if(error) for(x=0;x<dim;x++) tot_err=tot_err+error[x];
  tot_err+=edge_err;

  if(maxError<tot_err) maxError=tot_err;
  return tot_err;
}

  
double 
Token::err(int ind){

  gmt_assert(ind<dim);
  if(error)
    return error[ind];
  else
    return 0;
}


double
Token::totalErr(){
  return tot_err;
}


double
Token::edgeErr(){

  return edge_err;
}


void
Token::dump(){
  int x;

  cout << "t=( ";
  for(x=0;x<dim;x++){
    if(x>0) cout << " , ";
    cout << t[x];
  }
  cout << " ) ";

  

  if(error){
    cout << "| Err=( ";
    for(x=0;x<dim;x++){
      if(x>0) cout << " , ";
      cout << error[x];
    }
    cout << " ) ";
  }

  if(tot_err>0)
    cout << "   ERR: " << tot_err;

}


void
Token::write(FILE* file){
  int x,l;

  fwrite((char*)&dim,sizeof(int),1,file);
  fwrite((char*)t,sizeof(int),dim,file);
  

}

void
Token::read(FILE* file){
  int d;

  fread((char*)&d,sizeof(int),1,file);
  init(d);
  fread((char*)t,sizeof(int),dim,file);

}



