/********************************************************************
              Copyright 1994 University of Bern              
     
                   All Rights Reserved
 
Permission to use this software and its documentation for any purpose
 and without fee is hereby granted, provided that the above copyright 
notice appear in all copies and that both that copyright notice and this 
permission notice appear in supporting documentation. Do not distribute 
without specific, written prior permission.  
                                                  
Author: Bruno T. Messmer	                       
Email:  messmer@iam.unibe.ch	                   
********************************************************************/


#include"Multiindex.h"



MultiIndex::MultiIndex(){

    Root=NULL;
    depth=0;

}
  

MultiIndex::~MultiIndex(){

    clear();

}
 
void
MultiIndex::init(int d){

    gmt_assert(Root==NULL);
    depth=d;
    Root=new SortedList;

}



void 
MultiIndex::insert(void* E,INT_TYPE* keys, int d){

    int x;
    SortedList* H,*H2,T;
    List* L;
    
    int val;

    gmt_assert(d==depth);

    for(x=0;x<d;x++)
	T.insert(NULL,(int) keys[x],(double) keys[x]);
    
    H=Root;

    for(x=0;x<d;x++){
	T.removeTop(&val);


	if(x<(d-1)){
	    H2=(SortedList*) H->getValue((double) val);
	    if(!H2){
		H2=new SortedList;
		H->insert(H2,0,(double) val);
	    }
	    H=H2;
	}else{
	    L=(List*) H->getValue((double) val);
	    if(L==NULL){
		L=new List;
		L->insert(E);
		H->insert(L,0,(double) val);
	    }else{
		L->insert(E);
	    }
	}
    }
}
  

void* 
MultiIndex::get(INT_TYPE* keys, int d){

    int x;
    SortedList* H,*H2,T;
    List* L;
    int val;
    
    gmt_assert(d==depth);

    for(x=0;x<d;x++)
	T.insert(NULL,(int) keys[x],(double) keys[x]);

    H=Root;

    for(x=0;x<d;x++){
	T.removeTop(&val);

	if(x<(d-1)){
	    H2=(SortedList*) H->getValue((double) val);
	    if(!H2){
		return NULL;
	    }
	    H=H2;
	}else{
	    L=(List*) H->getValue((double) val);
	    if(!L)return NULL;
	    else return L;
	}
    }

}


int 
MultiIndex::count(){

  return 0;
}


void 
MultiIndex::clear(){

    if(Root){
	recursiveClear(Root,0);
	delete Root;
    }
    Root=NULL;
    depth=0;

}
  

void
MultiIndex::recursiveClear(SortedList* S, int d){
    int k;
    SortedList *S2;
    List *L;
    
    if(S==NULL) return;
    while(S->count()){
	if(d<depth-1){
	    S2=(SortedList*) S->removeTop(&k);
	    recursiveClear(S2,d+1);
	    delete S2;
	}else{
	    L=(List*) S->removeTop(&k);
	    delete L;
	}
    }
}

