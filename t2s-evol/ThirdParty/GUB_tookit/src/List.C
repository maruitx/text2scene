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

#include "List.h"
  

List::List(){

  Top=NULL;
  Last=NULL;
  Count=0;
  index=0;

#ifdef STUDIES
  statMax=0;
  statTotal=0;
#endif
}


List::~List(){
  
  while(Top){
    Last=Top->next;
    delete Top;
    Top=Last;
  }
  Top=NULL;
  Last=NULL;
  Count=0;
}

List::List(List &l){
  void* R;  
  
  Top=NULL;
  Last=NULL;
  Count=0;
  index=0;
  l.reset();
  
  while(R=l.getnext()) insert(R);

}


List
List::operator=(List &l){
  void* R;

  if(this==&l) return *this;
  
  while(Top){
    Last=Top->next;
    delete Top;
    Top=Last;
  }

  Top=NULL;
  Last=NULL;
  Count=0;
  index=0;

#ifdef STUDIES
  statMax=0;
  statTotal=0;
#endif 


  l.reset();
  
  while(R=l.getnext()) insert(R);

  return *this;
}

void List::insert(void* E){

  if(!Top){
    Top=new element;
    Top->EL=E;
    Top->next=NULL;
    Last=Top;
  }else{
    Last->next=new element;
    Last=Last->next;
    Last->EL=E;
    Last->next=NULL;
  }

  Count++;

#ifdef STUDIES
  statTotal++;
  if(Count>statMax) statMax=Count;
#endif

}



void List::insertAtTop(void* E){
  element *hpt;

  if(!Top){
    Top=new element;
    Top->EL=E;
    Top->next=NULL;
    Last=Top;
  }else{
    hpt=Top;
    Top=new element;
    Top->next=hpt;
    Top->EL=E;
  }

  Count++;

#ifdef STUDIES
  statTotal++;
  if(Count>statMax) statMax=Count;
#endif

}

void
List::mv(List &Dest){
  Dest.Top=Top;
  Dest.Last=Last;
  Dest.Count=Count;
  Dest.index=0;
#ifdef STUDIES
  statMax=0;
  statTotal=0;
#endif 
  Top=NULL;
  Last=NULL;
  Count=0;
  index=0;
}


void
List::reset(){
  index_ptr=Top;
  index=0;
}
  

void* 
List::remove(int ind){
  int x;
  void* E;
  element *ptr,*sptr;

  gmt_assert(ind<Count);
  reset();
  if(ind==0){
    ptr=Top;
    Top=Top->next;
    if(Count==1) Last=Top;
  }else{
    ptr=Top;
    for(x=0;x<ind-1;x++) ptr=ptr->next;
    sptr=ptr;
    ptr=ptr->next;
    sptr->next=ptr->next;
    if(Last==ptr) Last=sptr;
  }

  Count--;  
  E=ptr->EL;
  delete ptr;
  return E;

}


void
List::deleteCurrent(){
  element* hptr;
  hptr=index_ptr;
  gmt_assert(index_ptr!=NULL);
  if(pre_index_ptr==NULL){
    Top=index_ptr->next;
  }else{
    pre_index_ptr->next=index_ptr->next;    
  }
  index_ptr=index_ptr->next;
  Count--;

  // changed: 11.11.94
  index--;
  index_ptr=pre_index_ptr;

  delete hptr;
}


void* 
List::getnext(){

  if(Count==0) return NULL;
  
  if(index==0){
    pre_index_ptr=NULL;
    index_ptr=Top;
    index++;
    return Top->EL;
  }

  if(index<Count){
    pre_index_ptr=index_ptr;
    index_ptr=index_ptr->next;
    index++;
    return index_ptr->EL;
  }

  if(index>=Count){
    pre_index_ptr=index_ptr;
    index=0;
    index_ptr=NULL;
    return NULL;
  }    
}


void* 
List::get(int ind){
  int x;
  element *ptr;

  if(ind>=Count) return NULL;

  x=0;
  
  ptr=Top;
  while(x!=ind){x++; ptr=ptr->next;}
  return ptr->EL;
}


int 
List::count(){
  return Count;
}



void 
List::dump(char* types){
  int x;
  element* sptr;

  if(strstr(types,"Token")!=NULL){
    Token *tok;
    sptr=Top;
    for(x=0;x<Count;x++){
      tok=(Token*) sptr->EL;
      tok->dump();
      sptr=sptr->next;

    }
    if(Count>0) cout << "\n";
  }

}
  


  
int 
List::currentIndex(){
  return index;
}


void
List::clear(){
/** throw away everything without deleting the objects */

  while(Count>0) remove(0);
  

}

void
List::clearDeep(){
/** throw away everything without deleting the objects */
  void *pt;
  while(Count>0){
    pt=remove(0);
    if(pt) delete pt;
  }
}


/*********************************************/
/*       Sorted List: works with an array    */
/*********************************************/



#if 0




SortedList::SortedList(){
  int x;

  BIGFIRST=1;
  Top=NULL;
  Last=NULL;
  for(x=0;x<20;x++) {ILIST[x]=0;LIST[x]=NULL;}
  
  Count=0;
  index=0;

#ifdef STUDIES
  statMax=0;
  statTotal=0;
#endif

}


SortedList::~SortedList(){

  Count = 0;
}


void
SortedList::descending(){
  BIGFIRST=-1;
}



double
SortedList::TopValue(){

  return LIST[0]->value;
}


void
SortedList::insert(void* E, int key, double value){

  int c;
  sortelement *ptr,*sptr,*hptr;

#ifdef STUDIES
  statTotal++;
  if((Count+1)>statMax) statMax=Count+1;
#endif


  value=value*BIGFIRST;

  if(Count==0){
    Top=new sortelement;
    Top->EL=E;
    Top->key=key;
    Top->value=value;
    Top->next=NULL;
    
    index=1;
    LIST[0]=Top;
    HCount=0;
    ILIST[0]=1;
    Count=1;
  }else{



    c=search(value);
    ptr=LIST[c];


    sptr=NULL;

    while((ptr->next!=NULL)&&(value>ptr->value)){sptr=ptr;ptr=ptr->next;}
  
    hptr=new sortelement;
    hptr->EL=E;
    hptr->value=value;
    hptr->key=key;

    Count++;

    if(value<ptr->value){
      hptr->next=ptr;
      if(sptr==NULL){	
	LIST[c]=hptr;
      }else{
	
	sptr->next=hptr;
      }
    }else{
      hptr->next=ptr->next;
      ptr->next=hptr;
    }
    ILIST[c]++;

    split(c);

  } 
}


int
SortedList::search(double value){
  int t,b,x,c;
  
  
  t=0;
  b=HCount;
  x=HCount/2;
  
  while(b-1>t){
    if(value>LIST[x]->value){
      t=x;
    }else{
      b=x;
    }
    x=(b+t)/2;
    if(x==t) x++;

  }
  if(value>LIST[b]->value){
    c=b;
  }else{
    c=t;
  }
  
  return c;
  
}

#define SPLIT 1000

void
SortedList::split(int c){
  int l,x,s;
  sortelement *hptr;

  l=HCount;
  if(ILIST[c]>SPLIT){
    if(HCount>=(20-2)) return;
    
    for(x=HCount;x>c;x--){LIST[x+1]=LIST[x];ILIST[x+1]=ILIST[x];}

    hptr=LIST[c];
    s=0;
    while(s<SPLIT/2) {hptr=hptr->next;s++;}
    
    LIST[c+1]=hptr;
    ILIST[c+1]=ILIST[c]-s;
    ILIST[c]=s;
    HCount++;
  }
 }


void 
SortedList::shrink(sortelement *sptr,int c){
  int x;

  ILIST[c]--;
  
  if(sptr==NULL){
    LIST[0]=LIST[0]->next;
  }else if(LIST[c]==sptr){
    LIST[c]->next=sptr->next->next;
  }else{
    sptr->next=sptr->next->next;
  }

  if(ILIST[c]==0){
    for(x=c;x<HCount;x++) {LIST[x]=LIST[x+1];ILIST[x]=ILIST[x+1];}
    HCount--;
  }	
  
}

  
void* 
SortedList::removeKey(int key){

  int c;
  void* E;
  sortelement *ptr,*sptr;
  
  ptr=LIST[0];

  
  sptr=NULL;
  while((ptr!=NULL)&&(ptr->key!=key)){
    sptr=ptr;
    ptr=ptr->next;
  }
  
#if 0
  if(!ptr){printf("FATAL: SortList has no key:%d",key);assert(0);}
#else
  if(!ptr) return NULL;
#endif

  E=ptr->EL;
  
  c=search(ptr->value);
  shrink(sptr,c);
  delete ptr;
  Count--;
  return E;

}


int
SortedList::isKey(int key){

  
  sortelement *ptr,*sptr;
  
  ptr=LIST[0];

  
  sptr=NULL;
  while((ptr!=NULL)&&(ptr->key!=key)){
    sptr=ptr;
    ptr=ptr->next;
  }

  if(ptr!=NULL) return 1;
  else return 0;

}



void* 
SortedList::getnext(int* key){

  if(Count==0) return NULL;
  
  if(index==0){
    index_ptr=LIST[0];
    index++;
    *key=index_ptr->key;
    return index_ptr->EL;
  }

  if(index!=Count){
    index_ptr=index_ptr->next;
    index++;
    *key=index_ptr->key;
    return index_ptr->EL;
  }

  if(index==Count){
    index=0;
    index_ptr=NULL;
    return NULL;
  }    
}



void*
SortedList::get(int i){
  int x;
  sortelement *ptr;

  if(i>=Count) return NULL;

  x=0;
  
  ptr=LIST[0];
  while(x!=i){x++; ptr=ptr->next;}
  return ptr->EL;
  

}


void
SortedList::initNext(){
  index_ptr=Top;
  index=0;
}


void
SortedList::deleteCurrent(){
  sortelement* hptr;
  hptr=index_ptr;
  gmt_assert(index_ptr!=NULL);
  if(pre_index_ptr==NULL){
    Top=index_ptr->next;
  }else{
    pre_index_ptr->next=index_ptr->next;    
  }
  index_ptr=index_ptr->next;
  Count--;
  delete hptr;
}

void*
SortedList::removeTop(int* key){
  sortelement *ptr;
  void* E;


  gmt_assert(Count);
  *key=-1;
  if(Count==0) return NULL;


  ptr=LIST[0];
  shrink(NULL,0);
  *key=ptr->key;
  E=ptr->EL;
  Count--;
  delete ptr;
  return E;

}
  

void* 
SortedList::top(int* key){

  if(Count>0){
    *key=LIST[0]->key;
    return LIST[0]->EL;
  }else{
    *key=0;
    return NULL;
  }
}
 
 
int
SortedList::count(){
  return Count;
}

void
SortedList::showItems(){
  int x;
  sortelement *ptr;
  
  ptr=LIST[0];
  for(x=0;x<Count;x++){
    cout << ptr->key;
    cout << " ; ";
    ptr=ptr->next;
  }
}

void 
SortedList::dump(char* types){
  int x;
  sortelement* sptr;

  if(strstr(types,"Token")!=NULL){
    Token *tok;
    sptr=LIST[0];
    for(x=0;x<Count;x++){
      tok=(Token*) sptr->EL;
      tok->dump();
      sptr=sptr->next;
    }
    if(Count>0) cout << "\n";
  }

}



void
SortedList::clear(){
  int key;
  while(Count>0) removeTop(&key);
}
  


#else

/*********************************************/
/*       Sorted List: TREE IMPLEMENTATION    */
/*********************************************/



SortedList::SortedList(){
  int x;

  BIGFIRST=1;
  Top=NULL;
  Last=NULL;
  
  Count=0;
  index=0;

#ifdef STUDIES
  statMax=0;
  statTotal=0;
  statDepth=0;
#endif

}


SortedList::~SortedList(){
  clear();
  Count = 0;
}


void
SortedList::descending(){
  BIGFIRST=-1;
}



SortedList::SortedList(SortedList &s){
  void* Q;
  double v;
  int key;

  Top=NULL;
  Last=NULL;
  
  Count=0;
  index=0;

#ifdef STUDIES
  statMax=0;
  statTotal=0;
  statDepth=0;
#endif

  BIGFIRST=s.BIGFIRST;
  
  s.initNext();
  
  while(Q=s.getnext(&key,&v)){
    insert(Q,key,v);
  }
}


SortedList
SortedList::operator=(SortedList &s){
  int key;
  double v;
  void* Q;

  BIGFIRST=s.BIGFIRST;
  
  if(this==&s) return *this;

  clear();
#ifdef STUDIES
  statMax=0;
  statTotal=0;
  statDepth=0;
#endif

  Count=0;

  s.initNext();
  
  while(Q=s.getnext(&key,&v)){
    insert(Q,key,v);
  }

  return *this;
}


SortedList
SortedList::operator+=(SortedList &s){
  int key;
  double v;
  void* Q;


#ifdef STUDIES
  statMax=0;
  statTotal=0;
  statDepth=0;
#endif

  s.initNext();
  
  while(Q=s.getnext(&key,&v)){
    insert(Q,key,v);
  }

  return *this;
}


double
SortedList::TopValue(){
  if(Count>0)
    return ((double) BIGFIRST)*Last->value;
  else
    return 0;
}


int
SortedList::getValueKey(double value){
  sortelement *ptr;
#ifdef STUDIES
  int i;
  i=0;
#endif
  
  ptr=Top;

  while(ptr){
#ifdef STUDIES
    i++;
    if(statDepth<i) statDepth=i;
#endif

    if(ptr->value==value) return ptr->key;

    if(ptr->value>value){
      if(ptr->right) ptr=ptr->right;
      else break;
    }else{
      if(ptr->left) ptr=ptr->left;
      else break;
    }
  }
  if(ptr==NULL) return -1;
  if(ptr->value==value) return ptr->key;
  return -1;
}




void*
SortedList::getValue(double value){
  sortelement *ptr;
#ifdef STUDIES
  int i;
  i=0;
#endif
  
  ptr=Top;

  while(ptr){
#ifdef STUDIES
    i++;
    if(statDepth<i) statDepth=i;
#endif

    if(ptr->value==value) return ptr->EL;

    if(ptr->value>value){
      if(ptr->right) ptr=ptr->right;
      else break;
    }else{
      if(ptr->left) ptr=ptr->left;
      else break;
    }
  }
  
  if(ptr==NULL) return NULL;
  if(ptr->value==value) return ptr->EL;
  return NULL;
}




void
SortedList::insert(void* E, int key, double value){

  sortelement *pos;
  sortelement *ptr,*sptr,*hptr;

#ifdef STUDIES
  statTotal++;
  if((Count+1)>statMax) statMax=Count+1;
#endif


  value=value*BIGFIRST;
  
  if(Count==0){
    Top=new sortelement;
    Top->EL=E;
    Top->key=key;
    Top->value=value;
    Top->right=NULL;    
    Top->left=NULL;
    Top->ancestor=NULL;
    index=1;
    Count=1;
    Last=Top;
  }else{
    int lastchanged;
    lastchanged=0;

    ptr=search(value);



    if(ptr->value>value){
      if(ptr==Last) lastchanged=1;
      ptr->right=new sortelement;
      ptr->right->ancestor=ptr;
      ptr=ptr->right;
    }else{
      ptr->left=new sortelement;
      ptr->left->ancestor=ptr;
      ptr=ptr->left;
    }  

    ptr->value=value;
    ptr->key=key;
    ptr->EL=E;
    ptr->left=NULL;
    ptr->right=NULL;

    if(lastchanged) Last=ptr;

    Count++;
  }
}



sortelement*
SortedList::search(double value){
  sortelement *ptr;
#ifdef STUDIES
  int i;
  i=0;
#endif
  
  ptr=Top;

  while(ptr){
#ifdef STUDIES
    i++;
    if(statDepth<i) statDepth=i;
#endif

    if(ptr->value>value){
      if(ptr->right) ptr=ptr->right;
      else break;
    }else{
      if(ptr->left) ptr=ptr->left;
      else break;
    }
  }
  
  return ptr;
}




void
SortedList::split(int c){
  
  gmt_assert(0);

 }


void 
SortedList::shrink(sortelement *sptr,int c){

  gmt_assert(0);
}

  
void* 
SortedList::removeKey(int key){

  int c;
  void* E;
  sortelement *ptr;

  if(Top==NULL) return NULL;

  ptr=isKeyRec(key,Top);
  if(!ptr) return NULL;

  E=ptr->EL;

  removePtr(ptr);
     
  Count--;
  return E;

}


void* 
SortedList::removeElement(void* el){

  int c;
  void* E;
  sortelement *ptr;

  if(Top==NULL) return NULL;

  ptr=isVoidRec(el,Top);
  if(!ptr) return NULL;

  E=ptr->EL;

  removePtr(ptr);
     
  Count--;
  return E;

}

void
SortedList::removePtr(sortelement* ptr){
  sortelement *sptr,*hptr;
  
  sptr=ptr->ancestor;

  if(ptr->left==NULL){
    hptr=ptr->right;    
  }else{
    hptr=ptr->left;
    if(hptr->right){
      while(hptr->right!=NULL) hptr=hptr->right;
      hptr->ancestor->right=hptr->left;
      if(hptr->left) hptr->left->ancestor=hptr->ancestor;
      hptr->left=ptr->left;       
    }
    hptr->right=ptr->right;
    ptr->left->ancestor=hptr;
  }

  if(ptr->right) ptr->right->ancestor=hptr;
  
  if(sptr!=NULL){
    if(sptr->left==ptr) sptr->left=hptr;
    else  sptr->right=hptr;
  }else{
    Top=hptr;
  }
  
  if(hptr!=NULL) hptr->ancestor=sptr;
 
  if(Top==NULL){
    Last=NULL;
  }else{
    
    if(Last==ptr){
      if(hptr) Last=hptr;
      else Last=sptr;
      while(Last->right!=NULL) Last=Last->right;
    }
  }

  delete ptr;


}


int
SortedList::isKey(int key){
  sortelement *ptr,*sptr;
  
  ptr=Top;
  
  if(isKeyRec(key,Top)) return 1;
  else return 0;

}


void*
SortedList::getKeyData(int key){
 sortelement* ptr;
  
  ptr=isKeyRec(key,Top);
  if(ptr)
    return ptr->EL;
  else
    return NULL;

}


double
SortedList::getKey(int key){
  sortelement* ptr;
  
  ptr=isKeyRec(key,Top);
  if(ptr)
    return ptr->value;
  else
    return -1;
}


sortelement*
SortedList::isKeyRec(int key,sortelement* nod){
  sortelement* ptr;  

  if(nod==NULL) return NULL;
  if(nod->key==key) return nod;
  
  if(ptr=isKeyRec(key,nod->left)) return ptr;
  if(ptr=isKeyRec(key,nod->right)) return ptr;

  return NULL;
}



sortelement*
SortedList::isVoidRec(void* el,sortelement* nod){
  sortelement* ptr;  

  if(nod==NULL) return NULL;
  if(nod->EL==el) return nod;
  
  if(ptr=isVoidRec(el,nod->left)) return ptr;
  if(ptr=isVoidRec(el,nod->right)) return ptr;

  return NULL;
}



void
SortedList::deleteCurrent(){
  
  gmt_assert(old_index_ptr!=NULL);
  removePtr(old_index_ptr);
  old_index_ptr=NULL;
  Count--;

}


void* 
SortedList::getnext(int* key){
  double v;

  return getnext(key,&v);

}



void* 
SortedList::getnext(int* key,double* v){
  void* E;
  sortelement * ptr;
  int stop;

  *key=-1;
  
  if(Count==0) return NULL;
  
  if(index_ptr==NULL){
    return NULL;
  }

   
  *key=index_ptr->key;
  E=index_ptr->EL;
  old_index_ptr=index_ptr;

  *v=((double) BIGFIRST)*index_ptr->value;

  if(index_ptr->left!=NULL){
    index_ptr=index_ptr->left;
    
    while(index_ptr->right!=NULL) index_ptr=index_ptr->right;

  }else{

    // the right side has already been visited!
    // if index is left succesor then visit next ancestor!
    stop=0;
    while(index_ptr->ancestor!=NULL){
      if(index_ptr==(index_ptr->ancestor->right))
	stop=1;
      
      index_ptr=index_ptr->ancestor;
      if(stop==1) break;
    }

    if(stop==0){
      index_ptr=NULL;
    }
  }

  return E;
}



sortelement*
SortedList::getRec(int i,int &c,sortelement* akt){
  sortelement* ptr;

  ptr=NULL;

  if(akt->right!=NULL) ptr=getRec(i,c,akt->right);

  if(i==c) return ptr;

  c++;

  if(c==i) return akt;

  if(akt->left!=NULL) ptr=getRec(i,c,akt->left);

  return ptr;

}

void*
SortedList::get(int i,int& key,double& value){
  int x;
  sortelement *ptr;
  int j=-1;

  ptr=Top;
  
  if(Top==NULL) return NULL;
  ptr=getRec(i,j,Top);
  
  value=ptr->value;
  key=ptr->key;
  return ptr->EL;
}


void*
SortedList::get(int i,int& key){
  int x;
  sortelement *ptr;
  int j=-1;

  ptr=Top;
  
  if(Top==NULL) return NULL;
  ptr=getRec(i,j,Top);
  
  key=ptr->key;
  return ptr->EL;
 
}

void*
SortedList::get(int i){
  int x;
  sortelement *ptr;
  int j=-1;

  ptr=Top;

  if(Top==NULL) return NULL;
  ptr=getRec(i,j,Top);

  return ptr->EL;

}


void
SortedList::initNext(){
  index_ptr=Last;
  old_index_ptr=NULL;
  index=0;
}



void*
SortedList::removeTop(int* key){
  sortelement *ptr;
  void* E;


/** Top is actually Last, this is a relict, so dont get confused    **/
/** Last is always thevery rightmost leaf, delete is therefore easy **/
/** just take the left subtree and link it under ancestor->right    **/
/** like that, the balance is  better kept                          **/


  gmt_assert(Count);
  *key=-1;

  if(Count==0) return NULL;

  ptr=Last;

  if(ptr!=Top){ 
    ptr->ancestor->right=ptr->left;
    
    
    if(ptr->left==NULL){
      Last=ptr->ancestor;
      Last->right=NULL;
    }else{
      Last=ptr->left;
      Last->ancestor=ptr->ancestor;
      while(Last->right!=NULL) Last=Last->right;
    }
    gmt_assert(Last->right==NULL);
  }else{
    Top=ptr->left;
    Last=Top;
    if(Last!=NULL){
      Last->ancestor=NULL;
      while(Last->right!=NULL) Last=Last->right;
    }
  }

  E=ptr->EL;
  *key=ptr->key;

  delete ptr;

  Count--;
  return E;
}




  

void* 
SortedList::top(int* key){

  if(Count>0){
    *key=Last->key;
    return Last->EL;
  }else{
    *key=0;
    return NULL;
  }
}
 
 
int
SortedList::count(){
  return Count;
}

void
SortedList::showItems(){
  int x;
  sortelement *ptr;
  int j=-1;

  if(Count==0)
    cout << " No entries \n";
  else
    for(x=0;x<Count;x++){
      ptr=getRec(x,j,Top);
      cout << "Key:" << ptr->key << "  Val:" << ptr->value << " \n";
    }
  
}


void 
SortedList::dump(char* types){
  int x,key;
  sortelement* sptr;

  if(strstr(types,"Token")!=NULL){
    Token *tok;
    
    initNext();
    while(tok=(Token*) getnext(&key)){
      tok->dump();
    }
    if(Count>0) cout << "\n";
  }else{
    initNext();
    while(getnext(&key)){
      cout << "key: " << key << "\n";
    }
  }
}



void
SortedList::clear(){
  int key;
  while(Count>0) removeTop(&key);
}

void
SortedList::clearDeep(){
  int key;
  void* ptr;

  while(Count>0){
    ptr=removeTop(&key);
    if(ptr!=NULL) delete ptr;
  }
}

#endif


/**********************************************/
/*                    Stack                   */
/**********************************************/



Stack::Stack(){

  Top=NULL;
  Count=0;
}


Stack::~Stack(){
  stackelement *ptr;

  if(Top){
    ptr=Top->next;
    delete Top;
    Top=ptr;
  }
}


void
Stack::push(int key,void* Tok){

  stackelement *ptr;

  ptr = new stackelement;
  ptr->key=key;
  ptr->EL=Tok;
  ptr->next=NULL;
  

  if(Top){
    ptr->next=Top;
  }

  Top=ptr;
  Count++;

}
      

void *
Stack::pop(int* key){
 
  stackelement *ptr;
  void* Tok;
  
  if(empty()) return NULL;
  
  ptr = Top;
  Top=Top->next;

  *key=ptr->key;
  Tok=ptr->EL;
  
  delete ptr;
  Count--;
  return Tok;
}
    

void*
Stack::peek(int* key){

  stackelement *ptr;
  void* Tok;
  
  if(empty()) return NULL;
  
  ptr = Top;

  *key=ptr->key;
  Tok=ptr->EL;
  
  return Tok;
}
    


int 
Stack::count(){
  
  return Count;
}
    

int
Stack:: empty(){
  if(Count==0) return 1;
  else return 0;
}


/***********************************/
/*     HASHTABLE implementation    */
/***********************************/

Hash::Hash(){
  int x;
  Dimension=100;
  HT=new HashStruct*[Dimension];
  for(x=0;x<Dimension;x++)
    HT[x]=NULL;

  entries=0;
}

Hash::Hash(int dim){
  int x;
  Dimension=dim;
  HT=new HashStruct*[Dimension];
  for(x=0;x<Dimension;x++)
    HT[x]=NULL;
  entries=0;
}


Hash::~Hash(){
  clear();
  delete HT;
}


void
Hash::clear2(){
  int x;
  HashStruct *ptr,*hptr;

  for(x=0;x<Dimension;x++){
    ptr=HT[x];
    while(ptr){
      hptr=ptr;
      ptr=ptr->next;
      delete hptr;
    }
    HT[x]=NULL;
  }
  entries=0;

}


int
Hash::count(){
  return entries;
}


void*
Hash::get(int i){
  HashStruct* ptr;
  int x,c;
  c=-1;
  for(x=0;x<Dimension;x++){
    ptr=HT[x];
    while(ptr!=NULL){
      c++;
      if(c==i) return ptr->info;
      ptr=ptr->next;
    }
  }
  return NULL;
}



void
Hash::clear(){
  int x;
  HashStruct *ptr,*hptr;

  for(x=0;x<Dimension;x++){
    ptr=HT[x];
    while(ptr){
      hptr=ptr;
      ptr=ptr->next;
      if(hptr->info!=NULL) 
	delete hptr->info;
      delete hptr;
    }
    HT[x]=NULL;
  }
  entries=0;
}
  

void 
Hash::insert(int key){
  insert(NULL,key);
}

void 
Hash::insert(void* data,int key){
  int i;
  HashStruct *ptr;
  
  i=hashfunc(key); 
  ptr=new HashStruct;
  ptr->next=HT[i];  
  HT[i]=ptr;
  ptr->info=data;
  ptr->key=key;
  entries++;
}


  
void* 
Hash::remove(int key){
  int i;
  void* data;
  HashStruct* ptr,*hptr;
  i=hashfunc(key);
  
  ptr=HT[i];
  
  hptr=NULL;
  while(ptr!=NULL){
    if(ptr->key==key){
      if(hptr==NULL)
	HT[i]=ptr->next;
      else
	hptr->next=ptr->next;

      data=ptr->info;
      entries--;
      break;
    }
    hptr=ptr;
    ptr=ptr->next;
  }

  if(ptr!=NULL)
    delete ptr;
  else
    return NULL;

  return data;
}
 

int 
Hash::in(int key){
  int i;
  HashStruct* ptr;
  i=hashfunc(key);
  
  ptr=HT[i];
  while(ptr!=NULL){
    if(ptr->key==key) return 1;
    ptr=ptr->next;
  }
  return 0;
}

void* 
Hash::getKey(int key){
  
  int i,ret;
  HashStruct* ptr;
  i=hashfunc(key);
  
  ptr=HT[i];
  while(ptr!=NULL){
    if(ptr->key==key) return ptr->info;
    ptr=ptr->next;
  }
  return NULL;
}


#if 0
void
Hash::initKey(int key){
// if several items with identical keys exist
  
  int i,ret;
  HashStruct* ptr;
  i=hashfunc(key);
  
  ptr=HT[i];
  while(ptr!=NULL){
    if(ptr->key==key) {
      currentPtr=ptr;
      break;
    }
    ptr=ptr->next;
  }
  
}


void*
Hash::getnext(){
// in combination with initKey
  void* ret;

  if(currentPtr==NULL)
    return NULL;
  
  ret=currentPtr->info;

  currentPtr=currentPtr->next;
  while(currentPtr!=NULL){
    if(currentPtr->key==key)   break;
    ptr=ptr->next;
  }
  return ret;
}
#endif

int
Hash::hashfunc(int x){

  int i=x % Dimension;

  if(i>=0) 
      return i;
  else
      return -i;

}
