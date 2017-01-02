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

#include "Matcher.h"

//#include "TimerSO.h"

extern double Lookahead_result(Token* Original, int p0, int p1, Token* partial, AdjazenzMatrix *model, AdjazenzMatrix *image, int* AVAIL, AttributeClass* ATTC);


int ENV_LOOKAHEAD_LEVEL=0;
int ENV_UPPER_LEVEL=10000;
int ENV_EDGE_FORWARD_CHECKING_DISABLED=0;
double ENV_START_LOOKUP_AT_ERROR=0;

Matcher::Matcher(){

  cut_threshold=INFTY_COST;
  numOfModels=0;
  OPEN=NULL;
  OPENTREES=NULL;
  TERMINALS=NULL;
  RETURN_ON_FIND=0;
  char* heu;
  if(heu=getenv("ENV_LOOKAHEAD_LEVEL")) ENV_LOOKAHEAD_LEVEL=atoi(heu);
  if(heu=getenv("ENV_UPPER_LEVEL")) ENV_UPPER_LEVEL=atoi(heu);
  if(heu=getenv("ENV_START_LOOKUP_AT_ERROR")) ENV_START_LOOKUP_AT_ERROR=atof(heu);
  if(heu=getenv("ENV_EDGE_FORWARD_CHECKING_DISABLED")) ENV_EDGE_FORWARD_CHECKING_DISABLED=1;

}  


void 
Matcher::initAttributes(AttributeClass *A){

  ATT=A;

}


void
Matcher::setParameters(int* pset){
  int x;
  for(x=0;x<7;x++) MODES[x]=pset[x];
}


void
Matcher::clear(){
  int x;
  int key;
  MappingData* mp;


  if(numOfModels>0){

    deleteTrees();

    for(x=0;x<numOfModels;x++){
      delete OriginalModels[x];
    }
  }

  sumSteps=0;
  check_count=0;
  expandedNodes=0;
#ifdef STUDIES
 // for(x=0;x<MAX_MODEL_SIZE;x++){
 //   StatLevel[x]=0;
 // }
#endif

  StatLevelCount=0;
  numOfModels=0;

  while(Collection.count()>0){
    mp=(MappingData*) Collection.removeTop(&key);
    delete mp;
  }

  while(TotalCollection.count()>0){
    mp=(MappingData*) TotalCollection.removeTop(&key);
    delete mp;
  }


  Collection.clear();
  TotalCollection.clear();

}


void
Matcher::deleteSearchTree(){
  TNode** delarray;
  TNode* NX;
  int x,y;
  int key;

  delarray=new TNode*[OPEN->statTotal];
  x=0;
  
  while(Terminal->count()>0){
    NX=(TNode*) Terminal->remove(0);
    
//    assert(x<OPEN->statTotal);
    delarray[x]=NX;
    NX->level=-2;
    x++;
    marchBackwards(NX->parent,delarray,x);
  }

  while(OPEN->count()>0){
    NX=(TNode*) OPEN->removeTop(&key);

//    assert(x<OPEN->statTotal);
    delarray[x]=NX;
    NX->level=-2;
    x++;
    marchBackwards(NX->parent,delarray,x);
  }


  for(y=0;y<x;y++) delete delarray[y];
  delete delarray;

}


void
Matcher::marchBackwards(TNode* NX,TNode** delarray,int &x){

  if(NX==NULL) return;

  if(NX->level==-2) return;

//  assert(x<OPEN->statTotal);

  delarray[x]=NX;
  x++;
  NX->level=-2;
  marchBackwards(NX->parent,delarray,x);
}
  



void 
Matcher::setModelDataBase(ModelDataBaseType* MD,int count){
  int x,y;

  clear();

  MB=MD;
  numOfModels=count;

  for(x=0;x<numOfModels;x++){
    
    if(StatLevelCount<MB[x].G->AM.numberOfVertices()) StatLevelCount=MB[x].G->AM.numberOfVertices();

#if 0    

    OriginalModels[x]=new Token(MB[x].G->AM.numberOfVertices());

    for(y=0;y<MB[x].G->AM.numberOfVertices();y++){
      OriginalModels[x]->set(y,y);
    }
#else

    OriginalModels[x]=MB[x].G->AM.orderCoherent();

#endif

  }
}


void 
Matcher::deleteTrees(){
  int x;

  BESTTREE.clear();
  OPEN=NULL;
  if(OPENTREES==NULL) return;

  for(x=0;x<numOfModels;x++){
    OPEN=&OPENTREES[x];
    Terminal=&TERMINALS[x];
    deleteSearchTree();
  }
  
 
  Terminal=NULL;
  delete[] TERMINALS;
  delete[] OPENTREES;

  OPENTREES=NULL;
  TERMINALS=NULL;

}

void 
Matcher::match(AdjazenzMatrix* M){

  match(M,-1);

}

void 
Matcher::match(AdjazenzMatrix* M,double u_thres){
  int x,y,i;
  int *node;
  long ent,deletionTime;
  
  deletionTime=0;
  
 
    
  if(MODES[3]==1) ATT->setInsertionCost(0);

  AM=M;

  if(MODES[6]==2)
    AVAIL=new int[MB[0].G->numberOfVertices()+20];
  else
    AVAIL=new int[AM->numberOfVertices()];

#ifdef DEBUG
  maxError=0;
#endif

  if(u_thres==-1)
    THRESHOLD=30000000;
  else
    THRESHOLD=u_thres;

  deleteTrees();

  OPENTREES=new SortedList[numOfModels];
  TERMINALS=new List[numOfModels];
  
  for(x=0;x<numOfModels;x++){
    
    if(MB[x].G->AM.isGrain) 
      continue;
    
    Start=new TNode;
    Start->level=-1;
    Start->fferror=0;
    Start->parent=NULL;
    
    OPENTREES[x].clear();
    OPENTREES[x].insert(Start,-1,0);
    
    BESTTREE.insert(&OPENTREES[x],x,OPENTREES[x].TopValue());
    
  }
  

  if((MODES[1]!=1)||(MODES[2]==2)){

    startTime=elapsed_time();

    for(x=0;x<numOfModels;x++){

      CurrentModel=x;
      Original=OriginalModels[x];
      CurrentMatrix=&MB[CurrentModel].G->AM;
      
      if(MB[x].G->AM.isGrain) continue;
      
#ifdef DEBUGTREE
      for(i=0;i<Original->length();i++)
	printf("%4d",(*Original)[i]);
      printf("\n");
#endif
      
      if(MODES[1]==1){
	
#ifdef DEBUG
	// cout << "Searching Model " << x << ":\n";
	DeepestLevel=0;
	
#endif


	OPEN=&OPENTREES[x];
	if(MODES[2]==2) OPEN->descending();	
	Terminal=&TERMINALS[x];
	
	treeSearch();
	
	ent=elapsed_time();
		
	ent=elapsed_time()-ent;
	deletionTime+=ent;
	
	THRESHOLD=Collection.TopValue();
      }else{
	/** ULLMAN **/
	
	UllmansAlgo();
	
      }
    }

    endTime=elapsed_time()-deletionTime;

  }else{

    startTime=elapsed_time();

    AdjazenzMatrix *IAM=AM;
    Token* ImageVertices=AM->orderCoherent();


    while((BESTTREE.count()>0)&&(BESTTREE.TopValue()<=THRESHOLD)){
      int key;
    
      OPEN=(SortedList*) BESTTREE.removeTop(&key);
      CurrentModel=key;
      Terminal=&TERMINALS[key];  

      if(MODES[6]!=2){
	CurrentMatrix=&MB[CurrentModel].G->AM;
	Original=OriginalModels[key];
	 
      }else{
	CurrentMatrix=IAM;
	Original=ImageVertices;
	AM=&MB[CurrentModel].G->AM;
     
      }


      treeSearch();

      if(MODES[6]==2){
	AM=IAM;
	CurrentMatrix=&MB[CurrentModel].G->AM;
      }

      if(OPEN->count()>0)
	BESTTREE.insert(OPEN,key,OPEN->TopValue()); 
    }


    endTime=elapsed_time();

   
  }
    
  if(MODES[3]==1) ATT->setInsertionCost(1);
  delete AVAIL;
  AVAIL=NULL;

}


void
Matcher::matchContinue(double u_thres){

  if(MODES[3]==1) ATT->setInsertionCost(0);

  TotalCollection+=Collection;
  Collection.clear();

  if(u_thres==-1)
    THRESHOLD=30000000;
  else
    THRESHOLD=u_thres;

 if(MODES[6]==2)
    AVAIL=new int[MB[0].G->numberOfVertices()+20];
  else
    AVAIL=new int[AM->numberOfVertices()];
  
  gmt_assert((MODES[1]==1)&&(MODES[2]==1));

  startTime=elapsed_time();
  
  AdjazenzMatrix *IAM=AM;
  Token* ImageVertices=AM->orderCoherent();

  while((BESTTREE.count()>0)&&(BESTTREE.TopValue()<=THRESHOLD)){
    int key;
    
    OPEN=(SortedList*) BESTTREE.removeTop(&key);
    CurrentModel=key;
    Terminal=&TERMINALS[key]; 

    if(MODES[6]!=2){
      CurrentMatrix=&MB[CurrentModel].G->AM;
      Original=OriginalModels[key];        
    }else{
      CurrentMatrix=IAM;
      Original=ImageVertices;
      AM=&MB[CurrentModel].G->AM;
    }

#ifdef DEBUG
    // cout << "Searching Model " << key << ":\n";
#endif
    
    treeSearch();
    
    if(MODES[6]==2){
      CurrentMatrix=&MB[CurrentModel].G->AM;
      AM=IAM;
    }

    if(OPEN->count()>0)
      BESTTREE.insert(OPEN,key,OPEN->TopValue()); 
  }
  
  
  endTime=elapsed_time();
 
  if(MODES[3]==1) ATT->setInsertionCost(1);

  delete AVAIL;
  AVAIL=NULL;

}


int 
Matcher::NumberOfMatches(){
  return Collection.count();
}


MappingData* 
Matcher::query(int c){
  return (MappingData*) Collection.get(c);
}



/*************************************************/
/* Tree Search A* according to Nilsson           */
/*************************************************/
  

void
Matcher::treeSearch(){
  TNode* Next,*Son;
  int level,control;
  int i,node,x,o_node;
  double err,edge_err,future_error,top_err;
  AttId o_at,at;
  double bound;
  Token* partial=NULL;

  control=0;
  
  level=-1;


  if(MODES[2]==1)
    bound=OPEN->TopValue();
  else
    bound=30000000;

  while((OPEN->count()>0)&&(OPEN->TopValue()<=bound)){
    
    top_err=OPEN->TopValue();
    Next=(TNode*) OPEN->removeTop(&level);
    
    gmt_assert(level==Next->level);

#ifdef DEBUGTREE

    for(i=0;i<level;i++) printf("    ");
    printf("%4d\n",Next->image);

#endif  

    if(Next->fferror>THRESHOLD){
      delete Next;
      control=3;
    }

    if(control>0) break;
   
    if(Next->level==Original->length()-1){
      
      // instance found 
      verify(Next);

      collectMatch(Next);

      if(MODES[2]==1)
	THRESHOLD=Collection.TopValue();

      if(RETURN_ON_FIND) {
	THRESHOLD=-1;
	break;
      }

      continue;

    }

    level++;

#ifdef DEBUG
    if(level>DeepestLevel) {
   
      DeepestLevel=level;
    }
#endif

    for(x=0;x<AM->numberOfVertices();x++) AVAIL[x]=1;

    partial=followUp(Next);   // Set AVAIL[x] =0 if already used
    
    o_node=(*Original)[level];
    //o_at=MB[CurrentModel].G->AM.getNodeAttributeId(o_node);
    o_at=CurrentMatrix->getNodeAttributeId(o_node);


    for(x=0;x<AM->numberOfVertices();x++){
      if(isAvail(AVAIL[x],x)){
	node=x;
	at=AM->getNodeAttributeId(node);
	err=ATT->error(o_at,at);

	check_count++;
	if(err>=0){

	  //if(err>ATT->insertionCost(o_at)) continue;
	  
	    // change: 15.2.95
	    if((MODES[2]==1)||(err==0))
		edge_err=edgeError(node,o_node,Next);
	    else
		edge_err=0;

	  err=err+edge_err+Next->fferror;

      
	  if((err<=INFTY_COST)&&(err<=cut_threshold)){
	    if((MODES[2]==1)||(err==0)){
	      
	      


	      // LOOKING AHEAD FOR FUTURE COST ASSESMENT
	      partial->set(level,x);
	      AVAIL[x]=0;
	      
	      if((MODES[5]>1)&&(ENV_LOOKAHEAD_LEVEL<level)&&(ENV_UPPER_LEVEL>level)&&(err>=ENV_START_LOOKUP_AT_ERROR))
		future_error=Lookahead_result(Original,0, level+1, partial, CurrentMatrix, AM, AVAIL, ATT);
	      else
		future_error=0;

	      AVAIL[x]=1;

	      if(future_error+err<cut_threshold){
		
#ifdef STUDIES
		expandedNodes++;
		sumSteps+=level;
		StatLevel[level]++;
		if(err>maxError) maxError=err;
#endif	      
		
		//assert(top_err<=future_error+err);
				
		Son=new TNode;
		
		Son->level=level;
		Son->image=node;
		Son->original=(*Original)[level];
		Son->fferror=err;
		Son->parent=Next;
		
		if((MODES[3]>2)&&(Son->level==Original->length()-1)){
		  /* Isomorphism */
		  Son->fferror+=isomorphismError(Son);
		}
		
		if(MODES[2]==1)
		  OPEN->insert(Son,level,Son->fferror+future_error);
		if(MODES[2]==2)
		  OPEN->insert(Son,level,level);
	      }
	    }
	  }
	}
	AVAIL[x]=1;
      }
    }

    // delete the node o_node from the model
 
    if(MODES[2]==1){

      err=ATT->deletionCost(o_at);
      
      edge_err=edgeError(-1,o_node,Next);
      
      check_count++;
      err=err+edge_err+Next->fferror;

      if((err<INFTY_COST)&&(err<cut_threshold)){
	
	// LOOKING AHEAD FOR FUTURE COST ASSESMENT
	partial->set(level,-1);
	if((MODES[5]>1)&&(ENV_LOOKAHEAD_LEVEL<level)&&(ENV_UPPER_LEVEL>level)&&(err>=ENV_START_LOOKUP_AT_ERROR))
	  future_error=Lookahead_result(Original,0, level+1, partial, CurrentMatrix, AM, AVAIL, ATT);
	else
	  future_error=0;

	if(err+future_error<cut_threshold){
	  
#ifdef STUDIES
	  expandedNodes++;
	  sumSteps+=level;
	  StatLevel[level]++;
	  if(err>maxError) maxError=err;
#endif
	  
	  
	  Son=new TNode;
	  
	  Son->level=level;
	  Son->image=-1;
	  Son->original=(*Original)[level];
	  Son->fferror=err;
	  Son->parent=Next;
	  
	  if((MODES[3]>2)&&(Son->level==Original->length()-1)){
	    /* Isomorphism */
	    Son->fferror+=isomorphismError(Son);
	  }   
	  
	  OPEN->insert(Son,level,Son->fferror+future_error);
	}
      }
    }
    if(partial) delete partial;
    partial=NULL;
  }
}



int
Matcher::verify(TNode *leaf){

  return 1;
}


Token* 
Matcher::followUp(TNode *Next){
  TNode* temp;
  int *img;
  Token *tok;
  tok=new Token(Next->level+2);

  temp=Next;

  while(temp->level>=0){
    tok->set(temp->level,temp->image);
    if(temp->image>=0){
      AVAIL[temp->image]=0;
      
      if(temp->image>=AM->Dimension){
	int i=temp->image-AM->Dimension;
	Tsnode* sn=(Tsnode*) AM->SuperNodes.get(i);
	int* v;
	sn->parts.reset();
	while(v=(int*) sn->parts.getnext())
	  AVAIL[*v]=0;
	
      }
    }
    temp=temp->parent;
  }
  return tok;
}


int 
Matcher::isAvail(int avail, int x){
  
  if(!avail) return 0;

  if(x<AM->Dimension) return 1;
  int *v;
  x=x-AM->Dimension;
  Tsnode* sn=(Tsnode*) AM->SuperNodes.get(x);
  sn->parts.reset();
  
  while(v=(int*) sn->parts.getnext())
    if(!AVAIL[*v]) return 0;

  return 1;
}


double 
Matcher::edgeError(int node,int o_node,TNode* Next){
  TNode* temp;
  int *img;
  AdjazenzMatrix *A;
  double err,sumErr;
  AttId at,o_at;
  int edg,o_edg,found,o_node2;

  temp=Next;
  
  A=CurrentMatrix;
  // A=&MB[CurrentModel].G->AM;
  
  
#if 0
// Monomorphism
  
  if(MODES[3]==1){

    sumErr=0;
    
    while(temp->level>=0){
      
      o_node2=(*Original)[temp->level];
      
      A->initNext(o_node,o_node2);
      
      while((o_at=A->isNextEdgeAttId(&o_edg))!=NO_ATTRIBUTE){
	
	found=0;
	
	if((node>-1)&&(temp->image>-1)){
	  AM->initNext(node,temp->image);
          
	  while((at=AM->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	    err=ATT->error(at,o_at);
	    if(err>=0){
	      
/* substitutions being more costlier than deletions are not done! */
	      
	      if(err>ATT->insertionCost(o_at)) err=ATT->deletionCost(o_at);
	      
	      sumErr+=err;
	      found=1;
	      break;
	    }
	  }
	}
	
	if(found==0){
	  
	  err=ATT->insertionCost(o_at);
	  sumErr+=err;
	}
      }
      
// the other way around
      
      A->initNext(o_node2,o_node);
      
      while((o_at=A->isNextEdgeAttId(&o_edg))!=NO_ATTRIBUTE){
	
	found=0;
	
	if((node>-1)&&(temp->image>-1)){
	  AM->initNext(temp->image,node);
          
	  while((at=AM->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	    err=ATT->error(at,o_at);
	    if(err>=0){
	      sumErr+=err;
	      found=1;
	      break;
	    }
	  }
	}
	
	if(found==0){
	  
	  err=ATT->deletionCost(o_at);
	  sumErr+=err;
	}
      }
      
      temp=temp->parent;
    }
  }else{
  }
#endif

  sumErr=subIsoEdgeError(node,o_node,Next);
  
  
  return sumErr;
}



double
Matcher::subIsoEdgeError(int node,int o_node,TNode* Next){
  TNode* temp;
  int *img;
  AdjazenzMatrix *A;
  double err,sumErr;
  AttId at,o_at;
  int edg,o_edg,found,o_node2;
  AttId e_l[2],e_r[2];
  int dim_l,dim_r,x,y,z;
  double **m;
  
  temp=Next;
  
  A=CurrentMatrix;
  //A=&MB[CurrentModel].G->AM;
  sumErr=0;

  int rep;
  if(A->isDirected()) rep=2;
  else rep=1;

  while(temp->level>=0){    

    for(z=0;z<rep;z++){

    dim_l=0;dim_r=0;

    o_node2=(*Original)[temp->level];     
  
    if(z==0)
      A->initNext(o_node,o_node2); 
    else
      A->initNext(o_node2,o_node);

    while((o_at=A->isNextEdgeAttId(&o_edg))!=NO_ATTRIBUTE){	
      found=0;
      e_l[dim_l]=o_at;
      dim_l++;
    }
    e_l[dim_l++]=-1;
    
    if((node>-1)&&(temp->image>-1)){

      if(z==0)
	AM->initNext(node,temp->image);
      else
	AM->initNext(temp->image,node);

      while((at=AM->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	e_r[dim_r]=at;
	dim_r++;
      }
    }
    e_r[dim_r++]=-1;


#ifdef MULTIPLE_EDGES
    // no edges between these vertices:
    if(dim_l*dim_r>1){
      
      m=new double*[dim_l];
      for(x=0;x<dim_l;x++) m[x]=new double[dim_r];
      
      for(x=0;x<dim_l;x++)
	for(y=0;y<dim_r;y++)
	  if(e_r[y]==-1){
	    if(x<dim_l-1){
	      if(z==0)
		m[x][y]=ATT->deletionCostOfEdge(e_l[x],node,temp->image,(Graph*) AM->owner);
	      else
		m[x][y]=ATT->deletionCostOfEdge(e_l[x],temp->image,node,(Graph*) AM->owner);
	    }
	  }else if(e_l[x]==-1){
	    if(y<dim_r-1) m[x][y]=ATT->insertionCostOfEdge(e_r[y],1,1,NULL);
	  }else{
	    m[x][y]=ATT->error(e_l[x],e_r[y]);
	    if(m[x][y]<0) m[x][y]=INFTY_COST;
	  }
      
      sumErr+=HS(m,dim_l,dim_r);
      for(x=0;x<dim_l;x++) delete m[x];
      delete m;

      
    }    
    
    if(sumErr>=INFTY_COST) break;
    if(sumErr>=INFTY_COST) break;
    if(!A->isDirected()) break;
#else

  
    gmt_assert(dim_l<3);
    gmt_assert(dim_r<3);
  
    dim_l=1;
  
    if(!((e_r[0]==-1)&&(e_l[0]==-1))){
      if(e_r[0]==-1){
	if(z==0) 
	  sumErr += ATT->deletionCostOfEdge(e_l[0],node,temp->image,(Graph*) AM->owner); 
	else
	  sumErr += ATT->deletionCostOfEdge(e_l[0],temp->image,node,(Graph*) AM->owner);
      }else if(e_l[0]==-1){
	sumErr += ATT->insertionCostOfEdge(e_r[0],1,1,NULL);
      }else{
	double err=ATT->error(e_l[0],e_r[0]);
	if(err<0) err=INFTY_COST;
	sumErr += err;
      }
    }
    
    if((sumErr>=INFTY_COST)||(sumErr>=cut_threshold)) break;
    if(!A->isDirected()) break;

#endif  
  
  }
    if((sumErr>=INFTY_COST)||(sumErr>=cut_threshold)) break;
    temp=temp->parent;
  }
 
  return sumErr;

}

double
Matcher::isomorphismError(TNode* Son){
  int node;
  double err;
  AttId at;
  int *EX;
  int out,in,e,x;
 
  err=0;
  EX=new int[AM->numberOfEdges()];
  for(x=0;x<AM->numberOfEdges();x++) EX[x]=1;

  for(node=0;node<AM->numberOfVertices();node++){
    if((AVAIL[node])&&(Son->image!=node)){
      
      at=AM->getNodeAttributeId(node);
      err+=ATT->insertionCost(at);
      
      AM->initNext(node);
      while((e=AM->getnext(&out,&in,&at))!=-1){
	if(EX[e]==1){
	  EX[e]=0;
	  err+=ATT->insertionCostOfEdge(at,-1,-1,NULL);
	}
      }
    }
  }
  delete EX;
  return err;

}


void
Matcher::collectMatch(TNode* Next){
  TNode* temp;

  temp=Next;
  MappingData *MP;
  Token* tok;

  tok=new Token(Original->length());

  while(temp->level>=0){
    tok->set(temp->level,temp->image);
    temp=temp->parent;
  }

  tok->setEdgeErr(Next->fferror);

  MP=new MappingData;
  MP->setName(MB[CurrentModel].G->Name());
#if 0 
  MP->setNumber(MB[CurrentModel].key);
#else
  MP->setNumber(CurrentModel);
#endif
  MP->setMatch(tok,Original);

  delete tok;

  Collection.insert(MP,CurrentModel,Next->fferror);
  Terminal->insert(Next);

}


void
Matcher::collectUllmanMatch(int* H){
  MappingData *MP;
  Token* tok;
  int x;

  tok=new Token(Original->length());

  for(x=0;x<Original->length();x++){
    tok->set(x,H[x]);
  }

  MP=new MappingData;
  MP->setName(MB[CurrentModel].G->Name());
#if 0 
  MP->setNumber(MB[CurrentModel].key);
#else
  MP->setNumber(CurrentModel);
#endif
  MP->setMatch(tok,Original);

  delete tok;

  Collection.insert(MP,CurrentModel,0);

}


void 
Matcher::getStatistics(int *expansion,int* memory,long* time,int* instances,double* minError,int* checks){
  
  *expansion=expandedNodes;
  *memory=sumSteps;
  *time=endTime-startTime;
  *instances=Collection.count();
  *minError=Collection.TopValue();
  *checks=check_count;
}


int*
Matcher::distributionPartials(int &count){

  count=StatLevelCount;
  return StatLevel;
  
}


void
Matcher::statistic(char* name){
  int x,y,l,z,depth;
  FILE* file;
  char out[1024];
  char st[256];
  int *T,m;
  int sumTry,sumPassed,average;


//  if(OPEN==NULL) return;

#ifndef STUDIES

  cout << "Program was not compiled with the STUDIES variable set, try again\n";

#else

  file=fopen(name,"w");

  strcpy(out,"\n\n\n\n**************TREE SEARCH PERFORMANCE***************\n\n");
  fputs(out,file);

  depth=0;
  l=0;

  fputs("Number of expanded Nodes: ",file);

  sprintf(st,"%d",expandedNodes);
  strcpy(out,st);
  strcat(out,"\n");

  fputs(out,file);


  fputs("Number of Computation Steps: ",file);

  sprintf(st,"%d",sumSteps);
  strcpy(out,st);
  strcat(out,"\n\n");

  fputs(out,file);

  if(OPEN!=NULL){
    fputs("Max Entries in OPEN:",file);
    sprintf(st,"%d",OPEN->statMax);
    strcpy(out,st);
    strcat(out,"    Total Entries:");
    sprintf(st,"%d",OPEN->statTotal);
    strcat(out,st);
  }
  strcat(out,"\n");
  fputs(out,file);

  fputs("Max Error generated:",file);
  sprintf(st,"%f",maxError);
  strcpy(out,st);
  strcat(out,"  \n");
  fputs(out,file);

  strcpy(out,"\nTicks:");
  sprintf(st,"%d",time());
  strcat(out,st);
  fputs(out,file);

  fputs("\n\n",file);

  fclose(file);

#endif
}


int
Matcher::time(){
  return endTime-startTime;
}



/*** ULLMANS ALGORITHM ****/



void 
Matcher::UllmansAlgo(){
  int node,x,y,i,n,m;
  int*** FET;
  AttId at,oat;
  AdjazenzMatrix* MO;
  
  i=CurrentModel;
  

  MO=&MB[i].G->AM;  

  n=MO->numberOfVertices();
  
  m=AM->numberOfVertices();

 

  FET=new int**[n];
  
  

  for(x=0;x<n;x++){
    FET[x]=new int*[n];
    for(y=0;y<n;y++) FET[x][y]= new int[m];
  }


  for(x=0;x<n;x++) {
    node=(*Original)[x];
    at=MO->getNodeAttributeId(node);   
    for(y=0;y<m;y++){
      oat=AM->getNodeAttributeId(y);
      if(ATT->error(at,oat)==0){
	if(MO->degree(node)<=AM->degree(y)) FET[0][x][y]=1;
	else FET[0][x][y]=0;
      }else{
	FET[0][x][y]=0;
      }
    }
  }

  
  Ullman(0,MO,FET,n,m);

  for(x=0;x<n;x++){ 
    for(y=0;y<n;y++) delete FET[x][y];
    delete FET[x];
  }
  delete FET;

}



void 
Matcher::Ullman(int d,AdjazenzMatrix* MO,int*** FET,int n,int m){
  int RET;
  int node,x,y;
  int i,j;
 


  for(x=0;x<m;x++){

    if(FET[d][d][x]){

      AVAIL[d]=x;

#ifdef DEBUGTREE

    for(i=0;i<d;i++) printf("    ");
    printf("%4d\n",x);

#endif  

      
#ifdef STUDIES
      expandedNodes++;
      sumSteps+=d;
      StatLevel[d]++;
#endif


      if(d<n-1){
	for(i=d;i<n;i++)
//	  for(j=0;j<m;j++) FET[d+1][i][j]=FET[d][i][j];
	    memcpy(FET[d+1][i],FET[d][i],m*sizeof(int));

      }else{
//	cout << "Model " << CurrentModel << "  found\n";
	
	collectUllmanMatch(AVAIL);

	continue;
      }
  

      RET=refine(d+1,d,x,n,m,FET,MO);
      
      if(RET==0) continue;
      
      Ullman(d+1,MO,FET,n,m);
    }
  }
}




int
Matcher::refine(int feti,int d,int c,int n,int m,int*** FET,AdjazenzMatrix * MO){
  int x,y,z,k,L;
  int node,node2,ix,iz;
  int good,stillAlive;
  int out,in;
  AttId at,oat;

  for(x=d+1;x<n;x++) FET[feti][x][c]=0;


  /** Ullmanns Rule ***/
  /** Test Edges for the diferent positions **/


  /** if MODES[4]==2 then Ullman == A* == no lookahead **/

  if(MODES[4]==2) k=d+2;
  else k=n;

  if(MODES[4]==2) L=0;
  else L=d;


  for(x=d+1;x<k;x++){
    stillAlive=0;

    node=(*Original)[x];

    for(ix=0;ix<m;ix++){
      
      if(FET[feti][x][ix]){
	good=1;

	if(x==d+1)  check_count++;

/** only last change hast to be considered                 **/
/** but if MODES[4]==2 then all changes must be considered **/



	for(z=L;z<d+1;z++){

	  node2=(*Original)[z];
	  iz=AVAIL[z];

	  /** 0 direction */
	  
	  if(MO->isEdge(node,node2)!=-1){

	      MO->initNext(node,node2);

	      while((at=MO->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
		  good=0;
		  AM->initNext(ix,iz);
		  while((oat=AM->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
		      if(ATT->error(at,oat)==0){
			  good=1;
			  break;
		      }
		  }
		  if(!good) break;
	      }
	  }else{
	      if(AM->isEdge(ix,iz)!=-1)
		  good=0;
	  }
	  
	  

	  if(!good) break;
	  
	 
	  
	  if(!MO->isDirected()) continue;

	  /* 1 direction */

	  if(MO->isEdge(node2,node)!=-1){
	      MO->initNext(node2,node);
	      while((at=MO->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
		  good=0;
		  AM->initNext(iz,ix);
		  while((oat=AM->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
		      if(ATT->error(at,oat)==0){
			  good=1;
			  break;
		      }
		  }
		  if(!good) break;
	      }
	  }else{
	      if(AM->isEdge(iz,ix)!=-1)
		  good=0;
	  }

	  if(!good) break;
	}

	if(!good) FET[feti][x][ix]=0;
	else stillAlive=1;
      }
    }
    if(stillAlive==0) return 0;
  }
  return 1;
}



/*****************************/
/* Ullmans for InfoRetrieval */
/*****************************/


void
Matcher::UllmansAlgo(AdjazenzMatrix *M, Token* common, AdjazenzMatrix* COMMON, List* Unique, List* Ref){
  int node,x,y,i,n,m,k,cm,d;
  unsigned char**** FET;
  unsigned char*** CFET;
  AttId at,oat;
  AdjazenzMatrix* MO;
  Graph *GO;
  Token* tok;

  AM=M;
  n=AM->numberOfVertices();

  Original=AM->orderCoherent();
 

  k=numOfModels;
  AVAIL=new int[n];
  FET=new unsigned char***[n];

  for(d=0;d<n;d++){ // for each depth
    
    FET[d]=new unsigned char**[k];  // k models

    for(i=0;i<numOfModels;i++){  // for each model
      if(MB[i].G->AM.isGrain) continue;
      FET[d][i]=new unsigned char*[n];
      tok=(Token*) Unique->get(i);
      m=tok->length();

      for(x=0;x<n;x++){ // for each image vertex
	FET[d][i][x]=new unsigned char[m];
      }
    }
  }


 
  for(i=0;i<numOfModels;i++){ 
    if(MB[i].G->AM.isGrain) continue;
    
    MO=&MB[i].G->AM;
    tok=(Token*) Unique->get(i);
    m=tok->length();
    for(x=0;x<n;x++) { // for each vertex
      node=(*Original)[x];
      at=AM->getNodeAttributeId(node);   
      for(y=0;y<m;y++){
	oat=MO->getNodeAttributeId((*tok)[y]);
	if(ATT->error(at,oat)==0){
	  if(AM->degree(node)<=MO->degree((*tok)[y])) FET[0][i][x][y]=1;
	  else FET[0][i][x][y]=0;
	}else{
	  FET[0][i][x][y]=0;
	}
      }
    }
  }
 
  
  if(common!=NULL){
    m=common->length();
    CFET=new unsigned char**[n];
    for(x=0;x<n;x++){
      CFET[x]=new unsigned char*[n];
      for(y=0;y<n;y++) CFET[x][y]= new unsigned char[m];
    }
    

    for(x=0;x<n;x++) {
      node=(*Original)[x];
      at=AM->getNodeAttributeId(node);   
      for(y=0;y<m;y++){
	oat=COMMON->getNodeAttributeId((*common)[y]);
	if(ATT->error(at,oat)==0){
	  if(AM->degree(node)<=COMMON->degree((*common)[y])) CFET[0][x][y]=1;
	  else CFET[0][x][y]=0;
	}else{
	  CFET[0][x][y]=0;
	}
      }
    }
  }

  int* ACTIVE=new int[numOfModels+1];
  for(x=0;x<numOfModels+1;x++)
    ACTIVE[x]=1;
  
  Ullman(0,0,0,FET,CFET,common,COMMON,Unique,Ref,ACTIVE);
  
  for(x=0;x<n;x++){
    for(i=0;i<numOfModels;i++){
      if(MB[i].G->AM.isGrain) continue; 
      for(y=0;y<n;y++) delete[] FET[x][i][y];
      
      delete[] FET[x][i];
    }
    delete[] FET[x];
  }
  delete[] FET;

  if(common!=NULL){
    m=common->length();
    for(x=0;x<n;x++){
      for(y=0;y<n;y++) delete[] CFET[x][y];
      delete CFET[x];
    }
    delete[] CFET;
  }

  delete[] AVAIL;
  AVAIL=NULL;
  delete[] ACTIVE;
  delete Original;
}



void 
Matcher::Ullman(int flag,int ind, int d,unsigned char**** FET,unsigned char*** CFET, Token* common, AdjazenzMatrix* COMMON, List* Unique, List* Ref,int* ACTIVE){
  int RET;
  int node,x,y;
  int i,ii,iii,j;
  int n,m;
  unsigned char* oldACTIVE;
  AdjazenzMatrix *MO;
  Token* tok,*rtok;

  oldACTIVE=new unsigned char[numOfModels+1];

  if(flag==1){
    common=(Token*) Ref->get(0);
    COMMON=&MB[ind].G->AM;
  }
  
  for(ii=0;ii<numOfModels;ii++) oldACTIVE[ii]=ACTIVE[ii];
  
  n=AM->numberOfVertices();
  /* checking common structure */

  if(common!=NULL)
    m=common->length();
  else
    m=0;

  for(x=0;x<m;x++){
    
    if(CFET[d][d][x]){
      
      AVAIL[d]=x;
 
#ifdef STUDIES
      expandedNodes++;
      sumSteps+=d;
      StatLevel[d]++;
#endif

      
      if(d<n-1){
	for(i=d;i<n;i++)
	    for(j=0;j<m;j++) CFET[d+1][i][j]=CFET[d][i][j];

	if(flag){
	  MO=&MB[ind].G->AM;
	  tok=(Token*) Unique->get(0);
	  
	  for(y=0;y<tok->length();y++){
	    for(iii=d;iii<n;iii++)
	      for(j=0;j<tok->length();j++) FET[d+1][ind][iii][j]=FET[d][ind][iii][j];
	  }
	  
	}else{
	  for(ii=0;ii<numOfModels;ii++){
	    
	    if(MB[ii].G->AM.isGrain) continue;
	    if(!ACTIVE[ii]) continue;
	    MO=&MB[ii].G->AM;
	    tok=(Token*) Unique->get(ii);
	    
	    for(y=0;y<tok->length();y++){
	      for(iii=d;iii<n;iii++)
		for(j=0;j<tok->length();j++) FET[d+1][ii][iii][j]=FET[d][ii][iii][j];
	    }
	  }
	}
      }else{
//	cout << "Model " << -1 << "  found (c)\n";
	
	collectUllmanMatch(flag,AVAIL,0,COMMON,common,Unique,Ref);
	continue;
      }
      
      RET=refine(flag,ind,d+1,d,x,CFET,FET,COMMON,common,Unique,Ref,ACTIVE);
      
      if(RET) 
	Ullman(flag,ind,d+1,FET,CFET,common,COMMON,Unique,Ref,ACTIVE);
      
      
      for(ii=0;ii<numOfModels;ii++) ACTIVE[ii]=oldACTIVE[ii];
    }
  }
  
  

  
  for(x=0;x<numOfModels;x++){
    List tlist,rlist;
    
    if(MB[x].G->AM.isGrain) continue;
    MO=&MB[x].G->AM;
    
    if(flag){
      if(x!=ind) continue;
      tlist=(*Unique);
      rlist=(*Ref);
      rtok=(Token*) Ref->get(0);
      tok=(Token*) tlist.get(0);
      
    }else{
      if(!ACTIVE[x]) continue;
      tok=(Token*) Unique->get(x);
      rtok=(Token*) Ref->get(x);
      tlist.insert(tok);
      rlist.insert(rtok);
      
    }
    
    if(rtok!=NULL)
      m=rtok->length();
    else
      m=0;

    for(y=0;y<tok->length();y++){
      if(FET[d][x][d][y]){

	AVAIL[d]=y+m;

#ifdef STUDIES
	expandedNodes++;
	sumSteps+=d;
	StatLevel[d]++;
#endif


	if(d<n-1){
	  for(i=d;i<n;i++)
	    for(j=0;j<m;j++) CFET[d+1][i][j]=CFET[d][i][j];

	  for(i=d;i<n;i++)
	    for(j=0;j<tok->length();j++) FET[d+1][x][i][j]=FET[d][x][i][j];
	}else{
	//  cout << "Model " << x << "  found (u)\n";
	  collectUllmanMatch(1,AVAIL,x,COMMON,common,Unique,Ref);
	  continue;
	}
	
	RET=refine(1,x,d+1,d,y+tok->length(),CFET,FET,COMMON,common,&tlist,&rlist,ACTIVE);
	
	if(RET)
	  Ullman(1,x,d+1,FET,CFET,common,COMMON,&tlist,&rlist,ACTIVE);
	  
	
	for(ii=0;ii<numOfModels;ii++) ACTIVE[ii]=oldACTIVE[ii];
      }
    }
  }  
  delete [] oldACTIVE;
}




int
Matcher::refine(int flag,int model_ind,int feti,int d,int c,unsigned char*** CFET,unsigned char**** FET,AdjazenzMatrix * COMMON,Token* common, List* Unique,List* Ref,int* ACTIVE){
  int x,y,z,k,L;
  int node,node2,ix,iz;
  int good,stillAlive,stillAlive2,stillAlive3;
  int out,in;
  AttId at,oat;
  int n,m,ind,tm;
  Token* tok,*rtok,*current,*rtok2;
  AdjazenzMatrix *MO;

  stillAlive=0;
  
  n=AM->numberOfVertices();
  if(flag==0){
    current=common;
  }else{
    current=(Token*) Unique->get(0);
    common=(Token*) Ref->get(0);
    COMMON=&MB[model_ind].G->AM;
  }

  if(common!=NULL)
    m=common->length();
  else
    m=0;

  if(c<m)
    for(x=d+1;x<n;x++) CFET[feti][x][c]=0;
  else
    for(x=d+1;x<n;x++) FET[feti][model_ind][x][c-m];
  
  
  k=n;
  L=d;
  
  
  for(x=d+1;x<k;x++){
    stillAlive=0;
    
    node=(*Original)[x];

    
    for(ix=0;ix<m;ix++){
      
      if(CFET[feti][x][ix]){
	good=1;
	  
	if(x==d+1)  check_count++;
	
	for(z=L;z<d+1;z++){
	  
	  node2=(*Original)[z];
	  iz=AVAIL[z];
	  
	  if(iz>=m)
	    iz=iz-m;
	  
	  /** 0 direction */
	  
	  AM->initNext(node,node2);
	  
	  while((at=AM->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
	    good=0;
	    
	    
	    COMMON->initNext((*common)[ix],(*current)[iz]);
	    while((oat=COMMON->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
	      if(ATT->error(at,oat)==0){
		good=1;
		break;
	      }
	    }
	    if(!good) break;
	  }
	  
	  if(!good) break;
	  
	  /* 1 direction */
	  
	  AM->initNext(node2,node);
	  while((at=AM->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
	    good=0;
	    COMMON->initNext((*current)[iz],(*common)[ix]);
	    while((oat=COMMON->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
	      if(ATT->error(at,oat)==0){
		good=1;
		break;
	      }
	    }
	    if(!good) break;
	  }
	  
	  if(!good) break;
	}
	
	if(!good) CFET[feti][x][ix]=0;
	else stillAlive=1;
      }
    }
    
    // check unique vertices of each model

    stillAlive3=0;
    for(ind=0;ind<numOfModels;ind++){
      if(MB[ind].G->AM.isGrain) continue;
      stillAlive2=0;
      if(flag==1){
	if(ind!=model_ind) continue;
	tok=(Token*) Unique->get(0);
	rtok=(Token*) Ref->get(0);
	rtok2=rtok;
      }else{
	if(!ACTIVE[ind]) continue;
   
	tok=(Token*) Unique->get(ind);
	rtok=(Token*) Ref->get(ind);
	rtok2=rtok;
      }


      if(rtok!=NULL)
	m=rtok->length();
      else
	m=0;

      MO=&MB[ind].G->AM; 
      node=(*Original)[x];
      tm=tok->length();
      
      for(ix=0;ix<tm;ix++){
	
	if(FET[feti][ind][x][ix]){
	  good=1;
	  
	  if(x==d+1)  check_count++;
	  
	  for(z=L;z<d+1;z++){
	    
	    node2=(*Original)[z];
	    iz=AVAIL[z];
	
	    if(iz>=m){
	      iz=iz-m;
	      rtok=tok;
	    }else{
	      rtok=rtok2;
	    }
    
	    /** 0 direction */
	    
	    AM->initNext(node,node2);
	    
	    while((at=AM->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
	      good=0;
	      MO->initNext((*tok)[ix],(*rtok)[iz]);
	      while((oat=MO->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
		if(ATT->error(at,oat)==0){
		  good=1;
		  break;
		}
	      }
	      if(!good) break;
	    }
	    
	    if(!good) break;
	    
	    /* 1 direction */
	    
	    AM->initNext(node2,node);
	    while((at=AM->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
	      good=0;
	      MO->initNext((*rtok)[iz],(*tok)[ix]);
	      while((oat=MO->isNextEdgeAttId(&out))!=NO_ATTRIBUTE){
		if(ATT->error(at,oat)==0){
		  good=1;
		  break;
		}
	      }
	      if(!good) break;
	    }
	    
	    if(!good) break;
	  }
	  if(!good) FET[feti][ind][x][ix]=0;
	  else stillAlive2=1;
	}
      }
      
      if(!(stillAlive||stillAlive2))
	ACTIVE[ind]=0;
      else
	stillAlive3=1;
    }
    if(stillAlive3==0) return 0;
  }
  
  
  

  
  if(flag){
    if(ACTIVE[model_ind]){
      return 1;
    }else{
      return 0;
    }
  }else{
    for(x=0;x<numOfModels;x++){
      if(MB[x].G->AM.isGrain) continue;
      stillAlive+=ACTIVE[x];
    }
    if(stillAlive>0) return 1;
    else return 0;
  }
  gmt_assert(0);
}




void
Matcher::collectUllmanMatch(int flag,int* H,int m,AdjazenzMatrix* COMMON,Token* common,List* Unique,List *Ref){
  MappingData *MP;
  Token* tok,*rtok,*itok;
  int x,v,i,tm;

  
  
  for(i=0;i<numOfModels;i++){
    if(MB[i].G->AM.isGrain) continue;
    
    if(flag==1){
      if(i!=m) continue; 
      rtok=(Token*) Ref->get(0);
      itok=(Token*) Unique->get(0);
    }else{
      rtok=(Token*) Ref->get(i);
      itok=(Token*) Unique->get(i);
      
      if(rtok==NULL) continue;
    }
    
    if(rtok!=NULL)
      tm=rtok->length();
    else
      tm=0;

    tok=new Token(AM->numberOfVertices());
    
    for(x=0;x<AM->numberOfVertices();x++){
      if(H[x]>=tm)
	v=(*itok)[H[x]-tm];
      else
	v=(*rtok)[H[x]]; 
      tok->set(x,v);
    }
    
    MP=new MappingData;
    
    MP->setName(MB[i].G->Name());
    
    MP->setNumber(i);
    
    MP->setMatch(tok,Original);
    
    delete tok;
    
    Collection.insert(MP,CurrentModel,0);
  }
  
}
