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



/*NOTE**********************************************************


  There are two environment variables  which can be used to influence the 
  behavior of the Network based matcher:

  HEURISTIC1  and ENV_TERMINAL_DETECTION

  Use only HEURISTIC1 and set it to

  setenv HEURISTIC1  delta

  where delta is the smallest possible difference between to attributes
  (HEURISTIC1 may be 0)


  HEURISTIC2 : set to 0.95 in order prune the search prematurely

  We use a second lookahead heuristic called FUTerr_average and assign
  the maximum of this one and the old one to assignedFUTerr!
  
*****************************************************************/



#include "ComNet.h"
#include "gmt_assert.h"


extern double Lookahead_result(Token* Original, int p0, int p1, Token* partial, AdjazenzMatrix *model, AdjazenzMatrix *image, int* AVAIL, AttributeClass* ATTC);


static BridgeData BRIDGE_DUMMY;

extern int STOP_BIT;
int TERMINAL_DETECTION=0;

double HEURISTIC1=0;
int ENV_SECOND_HEURISTIC=0;
double HEURISTIC2=0;

#ifdef CONTROL_TEST
static List SUB_list,DEL_list,INS_list;
#endif

static Token DUMMY_TOK(1);
static Hash ACTIVE;

Token* DUMMY_TOKEN=&DUMMY_TOK;
/***************************************/
/*          NetWork Methods            */
/***************************************/


NetWork::NetWork(){
  COMPILER_MODUS=NORMAL;
  NNodeDim=0;
 
  CNodeDim=0;
  MNodeDim=0;
  NNodeX=0;
  CNodeX=0;
  MNodeX=0;

  NNodes=NULL;
  InfoNN=NULL;
  CNodes=NULL;
  InfoCN=NULL;

  MNodes=NULL;
  InfoMN=NULL;

  UIDLimit=0;
  attributeInit=0;

  CompileBest.descending();
  CurrentBestTry.descending();

  BREAKPOINT_SET=0;
  VX=NULL;
  EX=NULL;

  ModelData=NULL;
  ModelDataNumber=0;

  SUBGRAPH_ISO=0;
  callbackPtr=NULL;
  STOPPEDBYUSER=0;
  GRAIN=0;

  DEBUGSET=0;
  if(getenv("DEBUG")) DEBUGSET=1;

  POLLINGSET=0;
  if(getenv("POLLING")) POLLINGSET=1;

  ENV_CONTROL_STACK=0;
  char* heu;
  //HEURISTIC1=INFTY_COST;
  if(heu=getenv("HEURISTIC1")) HEURISTIC1=atof(heu);
  if(heu=getenv("HEURISTIC1")) HEURISTIC2=atof(heu);
  if(heu=getenv("ENV_CONTROL_STACK")) ENV_CONTROL_STACK=1;
  if(heu=getenv("ENV_TERMINAL_DETECTION")) TERMINAL_DETECTION=1;
  if(heu=getenv("ENV_SECOND_HEURISTIC")) ENV_SECOND_HEURISTIC=1;

  /* STATUS LINES */
  //cout << "*Environment Settings for Network*\n";
  //cout << "HEURISTIC1 is set to " << HEURISTIC1 << "\n";
  //cout << "HEURISTIC2 is set to " << HEURISTIC2 << "\n";
  //cout << "ENV_CONTROL_STACK is set to " << ENV_CONTROL_STACK << "\n";
  //cout << "ENV_TERMINAL_DETECTION is set to " << TERMINAL_DETECTION << "\n";
  //cout << "ENV_SECOND_HEURISTIC is set to " << ENV_SECOND_HEURISTIC << "\n";
  //cout << "DEBUG is set to " << DEBUGSET << "\n";


  cut_threshold=INFTY_COST;

  QueryList.descending();

  NetworkDepth=0;
  REMOVE_MODE=0;
  Late_Compile=0;

  Estimation_Series=NULL;

  STORE_IN_STOP=0;
  RETURN_ON_FIND=0;

  disjoint_test_func=NULL;

#ifdef STUDIES
  StatLevelCount=0;
#endif
}
  

NetWork::~NetWork(){
  discard();
}

void
NetWork::setModelDataBase(ModelDataBaseType* MDB,int num){
  
  ModelData=MDB;
  ModelDataNumber=num;

}


void
NetWork::lockModel(int i){

};




//void
//NetWork::setTimerForEstimation(Timer* Tx,Series_Type* Et){
//  Estimation_Series=Et;
//  Timex=Tx;
//}


void 
NetWork::initAttributes(AttributeClass *ATT){
  attributeInit=1;
  ATTC=ATT;  
}


int
NetWork::write(char* name){
  char name_info[256];
  FILE *file;
  int x;

  file=fopen(name,"w");
  
  fwrite((char*)&UIDLimit,sizeof(int),1,file);

  fwrite((char*)&NNodeX,sizeof(int),1,file);
  fwrite((char*)&CNodeX,sizeof(int),1,file);
  fwrite((char*)&MNodeX,sizeof(int),1,file);

  for(x=0;x<NNodeX;x++) NNodes[x].write(file);
  for(x=0;x<CNodeX;x++) CNodes[x].write(file);
  for(x=0;x<MNodeX;x++) MNodes[x].write(file);

  fclose(file);

  strcpy(name_info,name);
  strcat(name_info,"_info");
  
  file=fopen(name_info,"w");
  
  fwrite((char*)&NNodeX,sizeof(int),1,file);
  fwrite((char*)&CNodeX,sizeof(int),1,file);
  fwrite((char*)&MNodeX,sizeof(int),1,file);

  for(x=0;x<NNodeX;x++) InfoNN[x].write(file);
  for(x=0;x<CNodeX;x++) InfoCN[x].write(file);
  for(x=0;x<MNodeX;x++) InfoMN[x].write(file);
  
  fclose(file);
  
  return 1;
}
  

int 
NetWork::read(char* name){
  int x;  
  
  char name_info[256];
  FILE *file;
  file=fopen(name,"r");

  clearNet();
  discard();
  
  fread((char*)&UIDLimit,sizeof(int),1,file);

  fread((char*)&NNodeX,sizeof(int),1,file);
  fread((char*)&CNodeX,sizeof(int),1,file);
  fread((char*)&MNodeX,sizeof(int),1,file);

  NNodes=new NNodeType[NNodeX];
  CNodes=new CNodeType[CNodeX];
  MNodes=new MNodeType[MNodeX];

  NNodeDim=NNodeX;
  CNodeDim=CNodeX;
  MNodeDim=MNodeX;

  for(x=0;x<NNodeX;x++) NNodes[x].read(file);
  for(x=0;x<CNodeX;x++) CNodes[x].read(file);
  for(x=0;x<MNodeX;x++) MNodes[x].read(file);

  fclose(file);

  strcpy(name_info,name);
  strcat(name_info,"_info");
  
  file=fopen(name_info,"r");
  
  fread((char*)&NNodeX,sizeof(int),1,file);
  fread((char*)&CNodeX,sizeof(int),1,file);
  fread((char*)&MNodeX,sizeof(int),1,file);

  InfoNN=new NetInfoType[NNodeX];
  InfoCN=new NetInfoType[CNodeX];
  InfoMN=new NetInfoType[MNodeX];



  for(x=0;x<NNodeX;x++) InfoNN[x].read(file);
  for(x=0;x<CNodeX;x++) InfoCN[x].read(file);
  for(x=0;x<MNodeX;x++) InfoMN[x].read(file);
  
  fclose(file);
  
  return 1;
}


void
NetWork::setMode(int m){
  mode=m;
}



void
NetWork::setGrainInNetwork(){
  GRAIN=1;
}

  



void 
NetWork::setData(AdjazenzMatrix *M){
  int d,x;

  WModel=M;

  if(VX){
    if(WMDim<M->numberOfVertices()){
      delete VX;
      VX=new int[M->numberOfVertices()];
    }
  }else{
    VX=new int[M->numberOfVertices()];
  }

  if(EX)
    delete EX;

  WMDim=M->numberOfVertices();
  d=M->numberOfEdges();
 
  EX=new int[d];

  for(x=0;x<WMDim;x++) VX[x]=-1;
  
  if(mode==COMPILE_MODE){
    d=M->numberOfEdges();
 
    for(x=0;x<d;x++) EX[x]=-1;

    CompileBalanced.clear();
    CompileBest.clear();
    
  }
}
  
  
void 
NetWork::compileForSGI(AdjazenzMatrix *M,int num){

 
  COMPILER_MODUS=BALANCED;
  SUBGRAPH_ISO=1;
  compile(M,num);
 
}



void 
NetWork::compileBalanced(AdjazenzMatrix *M,int num){
 

  MODES[1]=BALANCED;
  COMPILER_MODUS=BALANCED;
  compile(M,num);
 
}


void 
NetWork::setParameters(int* p){
  int x;
  for(x=0;x<6;x++) MODES[x]=p[x];
}



void
NetWork::registerCallback(void* fptr){
  
  callbackPtr=(void (*)(NetWork* N)) fptr;

}


void
NetWork::polling(){

  if(callbackPtr!=NULL)
    callbackPtr(this);

}




void
NetWork::lateCompile(AdjazenzMatrix *M,int num){
  action_element *P;
  int x,d,key,i;
  Token *tok;
  int oldNNodeX,oldMNodeX;
  AdjazenzMatrix *CurrentImage;
  
  /*
    lateCompile may only be started if there is still a valid Image matrix present
    */
  
  for(x=0;x<NNodeX;x++)
    NNodes[x].GO.mv(NNodes[x].TMP_GO);
  
  for(x=0;x<CNodeX;x++)
    CNodes[x].GO.mv(CNodes[x].TMP_GO);
  
  oldNNodeX=NNodeX;
  oldMNodeX=MNodeX;
  
  CurrentImage=WModel;
  i=Token::inst_count;
  Late_Compile=1;
  
  compile(M,num);
  
  Late_Compile=0;
  // Token::inst_count=i;
  
  
  for(x=0;x<NNodeX;x++)
    if(NNodes[x].GO.count()>0)
      NNodes[x].GO.clear();
  
  for(x=0;x<CNodeX;x++)
    if(CNodes[x].GO.count()>0)
      CNodes[x].GO.clear();
  
  
  setMode(DEBUG_MODE);
  setData(CurrentImage);
/********* Send orignal image through to new NNodes *******/
         
  
  int v;
  
  for(v=0;v<WModel->numberOfVertices();v++){
    if(Hidden.in(v)) continue;
    tok=new Token(1);
    tok->set(0,v);
    for(x=oldNNodeX;x<NNodeX;x++){
      P=new action_element;
      P->node=x;
      P->type=NNODE_TYPE;
      P->tok=tok;
      P->index=0;
      ACTION.push(TREAT,P);
    }
    process();  
    delete tok;
  }
 
  

  UPDATE_LIST.initNext();
  update_element *up;
  while((up=(update_element*) UPDATE_LIST.getnext(&key))!=NULL){
    while(up->tmp_go->count()>0){
      Token* tok=(Token*) up->tmp_go->remove(0);
      ruleMove(up->node,up->type,tok,NULL,&up->left,&up->right,NULL);
      
      /* work on stack */
      process();
    }
  }
  
  for(x=0;x<NNodeX;x++)
    if(NNodes[x].TMP_GO.count()>0)
      NNodes[x].TMP_GO.mv(NNodes[x].GO);

  for(x=0;x<CNodeX;x++)
    if(CNodes[x].TMP_GO.count()>0)
      CNodes[x].TMP_GO.mv(CNodes[x].GO);
  
  


/**************Update the FUTerr and assigned errors *****/

  
  tok=new Token(1);
  tok->set(0,0);
  
  ERROR_ESTIMATION=1;
  for(x=oldNNodeX;x<NNodeX;x++){
    P=new action_element;
    P->node=x;
    P->type=NNODE_TYPE;
    P->tok=tok;
    P->index=0;
    ACTION.push(TREAT,P);
  }
  
  process();
  delete tok;
  while(UPDATE_LIST.count()>0){
    up=(update_element*) UPDATE_LIST.removeTop(&key);
    tok=NNodes[up->node].FutErrorToken;
    ruleMove(up->node,up->type,tok,NULL,&up->left,&up->right,NULL);
  }
  
  process();

  /****** assign new FUT errors from below ***/

  ConnectType* CP,C;

  UPDATE_ESTIMATION=1;
  for(x=oldMNodeX;x<MNodeX;x++){ 
    MNodes[x].parents.reset();
    while(CP=(ConnectType*) MNodes[x].parents.getnext()){
      C=*CP;
      C.direction=0;
      LECRassignFUTrec(&C,0);
    }
  }
  UPDATE_ESTIMATION=0;
  ERROR_ESTIMATION=0;

}



void 
NetWork::compile(AdjazenzMatrix *M,int num){
  int x,y,*m;
  ConnectType EndNode;
  ConnectType *CB;
  int fo,edg;
  AttId at;
  Token *orderToken;

  gmt_assert(attributeInit);
  
  setMode(COMPILE_MODE);
  setData(M);
  WMNumber=num;

#ifdef STUDIES
  if(StatLevelCount<WMDim) StatLevelCount=WMDim;
#endif

 
  reallocNodes();

  fo=0;
  if(MODES[1]==BALANCED) fo=1;
  MODES[1]=NORMAL; //NET

  orderToken=WModel->orderCoherent();

  for(x=0;x<WMDim;x++) run(x);
  
  if(fo) MODES[1]=BALANCED; //NET

  int x2;
  for(x2=0;x2<WMDim;x2++){
    x=(*orderToken)[x2];
    if(VX[x]==-1){   /* Node is not yet in the net */
      ConnectType *CB;

      NNodes[NNodeX].TestID = WModel->getNodeAttributeId(x);
      NNodes[NNodeX].totalDeletionCost=ATTC->deletionCost(NNodes[NNodeX].TestID); 
      WModel->initNext(x,x);
      while((at=WModel->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	BridgeData* bd;
	bd=new BridgeData;
	bd->n1=x;
	bd->n2=x;
	bd->id=at;
	NNodes[NNodeX].totalDeletionCost+=ATTC->deletionCost(bd->id);
	NNodes[NNodeX].testList.insert(bd);
      }

 
      NNodes[NNodeX].TopNumber=0;
      NNodes[NNodeX].UID=UIDLimit;
      UIDLimit++;
      
      int y2;
      for(y2=x2;y2<WMDim;y2++){
	y=(*orderToken)[y2];
	if(VX[y]==-1){
	  Token* dtok;
	  Token* tok;
	  
	  tok=new Token(1);
	  tok->set(0,y);
	  if(dtok=NTest(NNodeX,tok)){
	    delete dtok;
	    NNodes[NNodeX].GO.insert(tok);
	    VX[y]=NNodes[NNodeX].UID;
	  }else{
	    delete tok;
	  }
	}
      }

      CB=new ConnectType;
      CB->NType=NNODE_TYPE;
      CB->ind=NNodeX;
      CompileBest.insert(CB,NNodes[NNodeX].UID,NNodes[NNodeX].TopNumber);

      NNodeX++;
    }
  }  /** All Nodes of WModel are now in the Net  **/

  delete orderToken;

  /** Look for Egdes and pass them until a CNodes with a comlete Edge **/
  /** Edge List is found                                              **/

  if(MODES[1]==BALANCED) //NET
    setBalancedList();
  else
    CompileBalanced.clear();
  
  
  while((CompileBest.count()>0)||(CompileBalanced.count()>0)){
    int key,j;
    ConnectType *C,*NC;
    Token *tok,*etok;
    action_element* P,*NQ;

    if(MODES[1]==NORMAL){  //NET
      C=(ConnectType*) CompileBest.removeTop(&key);
    }else if(MODES[1]==BALANCED){  //NET
      C=(ConnectType*) CompileBalanced.removeTop(&key);      
    }

    if(C->NType==NNODE_TYPE){
      
      if(!NNodes[C->ind].GO.count()) {
	delete C;C=NULL;
	continue;
      }

      j=0;
      if(MODES[1]==NORMAL){  //NET
	tok=(Token*) NNodes[C->ind].GO.get(j);

      }else{
	while(j<NNodes[C->ind].GO.count()){
	  tok=(Token*) NNodes[C->ind].GO.get(j);
		
	  if(isUsedInBalanced(tok,NNodes[C->ind].UID)) break;
	  j++;
	}
	if(j>=NNodes[C->ind].GO.count()){
	  delete C;C=NULL;
	  continue;
	}
      }


      if(isComplete(tok)){
	EndNode.NType=C->NType;
	EndNode.ind=C->ind;
	delete C;
	break;
      }

      if(DEBUGSET){
	cout << "\nbefore findPartial\n";
	debugDump();
      }

      List Elist;
      P=findPartial(tok,C,&Elist);
      NQ=createNode(P,tok,&Elist);
      delete P;
      Elist.clear();

      if(CNodes[NQ->node].TopNumber<NNodes[C->ind].TopNumber+1) CNodes[NQ->node].TopNumber=NNodes[C->ind].TopNumber+1;


      if(DEBUGSET){
	cout << "CNode:" << CNodeX;
      }
      
      NC=new ConnectType;
      NC->NType=NQ->type;
      NC->ind=NQ->node;
      NC->direction=LEFT;
      NNodes[C->ind].left_successor.insert(NC);
      NNodes[C->ind].represented++;
      if(NNodes[C->ind].TMP_GO.count()>0){
	// do update later
	insertIntoUPDATE(C->ind,C->NType,NNodes[C->ind].UID, &NNodes[C->ind].TMP_GO, NC,0);
      }

    }else{

      if(!CNodes[C->ind].GO.count()){
	delete C;C=NULL;
	continue;   
      }
  
      j=0;
      if(MODES[1]==NORMAL){  //NET
	tok=(Token*) CNodes[C->ind].GO.get(0);

      }else{
	while(j<CNodes[C->ind].GO.count()){
	  tok=(Token*) CNodes[C->ind].GO.get(j);

	  if(isUsedInBalanced(tok,CNodes[C->ind].UID)) break;
	  j++;
	}

	if(j>=CNodes[C->ind].GO.count()){
	  delete C;C=NULL;
	  continue;
	}
      }


      if(isComplete(tok)){
	EndNode.NType=C->NType;
	EndNode.ind=C->ind;
	delete C;
	break;
      }

      if(DEBUGSET){
          cout << "\nbefore findPartial\n";
	  debugDump();   
	};

      List Elist;
      P=findPartial(tok,C,&Elist);
      NQ=createNode(P,tok,&Elist);
      delete P;
      Elist.clear();

      if(CNodes[NQ->node].TopNumber<CNodes[C->ind].TopNumber+1) CNodes[NQ->node].TopNumber=CNodes[C->ind].TopNumber+1;

      if(DEBUGSET){
        cout << "CNode:" << CNodeX;
      }
     
      NC=new ConnectType;
      NC->NType=NQ->type;
      NC->ind=NQ->node;
      NC->direction=LEFT;
      CNodes[C->ind].left_successor.insert(NC);
      CNodes[C->ind].represented++;
      if(CNodes[C->ind].TMP_GO.count()>0){
	// do update later
	insertIntoUPDATE(C->ind,C->NType,CNodes[C->ind].UID, &CNodes[C->ind].TMP_GO, NC,0);
      }
    }
    
    CNodes[NQ->node].parents[0].NType=C->NType;
    CNodes[NQ->node].parents[0].ind=C->ind;
    CNodes[NQ->node].parents[0].direction=LEFT;
    

    TetMarch(NQ->node);
    
    if(MODES[1]==BALANCED) markTokenForBalanced(NQ->type,NQ->node); //NET

    /** place parent nodes in the Compilation list if they are still valid  */


    if(C->NType==CNODE_TYPE){
      CNodes[NQ->node].totalEdges+=CNodes[C->ind].totalEdges;
      CNodes[NQ->node].totalDeletionCost+=CNodes[C->ind].totalDeletionCost;
    }else{
      CNodes[NQ->node].totalEdges+=NNodes[C->ind].totalEdges;
      CNodes[NQ->node].totalDeletionCost+=NNodes[C->ind].totalDeletionCost;
    }


    if(!(MODES[1]==BALANCED)){ //NET
      if(C->NType==NNODE_TYPE){
	if(NNodes[C->ind].GO.count()){
	  CompileBest.insert(C,NNodes[C->ind].UID,NNodes[C->ind].TopNumber);
	}else{
	  delete C;C=NULL;
	}
      }else{
	if(CNodes[C->ind].GO.count()){
	  int l;
	  l=(*((Token*) CNodes[C->ind].GO.get(0))).length();
	  // CompileBest.insert(C,CNodes[C->ind].UID,CNodes[C->ind].TopNumber);
	  CompileBest.insert(C,CNodes[C->ind].UID,CNodes[C->ind].newLength);
	}else{
	  delete C;C=NULL;
	}
      }
    }else{
      if(!setPartBalancedList(C)){ delete C;C=NULL;}
    }


    /** place new CNodes in the Compilation List  **/

    C=new ConnectType;
    C->ind=NQ->node;
    C->NType=NQ->type;
    gmt_assert(CNodes[NQ->node].GO.count());

    if(MODES[1]==BALANCED){ //NET
      if(!setPartBalancedList(C)) {delete C;C=NULL;}
    }else{
      int l;
      l=(*((Token*) CNodes[C->ind].GO.get(0))).length();
      //     CompileBest.insert(C,CNodes[NQ->node].UID,CNodes[NQ->node].TopNumber);
      CompileBest.insert(C,CNodes[NQ->node].UID,CNodes[NQ->node].newLength);
    }

    if(NQ) delete NQ;

  }  /** continue with while **/


  /** build new model node  with EndNode **/


  if(GRAIN==0){
    ConnectType* CP;
  
    MNodes[MNodeX].modelNr=WMNumber;
    MNodes[MNodeX].UID=UIDLimit;
    UIDLimit++;
    CP=new ConnectType;
    CP->NType=EndNode.NType;
    CP->ind=EndNode.ind;
    MNodes[MNodeX].parents.insert(CP);

    if(EndNode.NType==NNODE_TYPE){
      Token *orig,*eorig;
      int *CI;
    
      
      gmt_assert(NNodes[EndNode.ind].GO.count()==1);
      
      orig=(Token*) NNodes[EndNode.ind].GO.remove(0);
      MNodes[MNodeX].Original=*(orig);
      MNodes[MNodeX].TopNumber=2;
     
      InfoNN[EndNode.ind].originalNodes.insert(orig);

      m=new int;
      *m=WMNumber;
      InfoNN[EndNode.ind].originalModels.insert(m);
      
      eorig=new  Token;
      (*eorig)=(*orig);
      InfoMN[MNodeX].originalEdges.insert(eorig);
      
      CI=new int;
      *CI=MNodeX;
      NNodes[EndNode.ind].models.insert(CI);
      
    }else{
      Token *orig,*eorig;
      int *CI;
      
      gmt_assert(CNodes[EndNode.ind].GO.count()==1);
      
      orig=(Token*) CNodes[EndNode.ind].GO.remove(0);
      MNodes[MNodeX].Original=*(orig);
      MNodes[MNodeX].TopNumber=CNodes[EndNode.ind].TopNumber;
      
      InfoCN[EndNode.ind].originalNodes.insert(orig);
      
      m=new int;
      *m=WMNumber;
      InfoCN[EndNode.ind].originalModels.insert(m);

      eorig=new  Token;
      (*eorig)=(*orig);
      InfoMN[MNodeX].originalEdges.insert(eorig);
    
      CI=new int;
      *CI=MNodeX;
      CNodes[EndNode.ind].models.insert(CI);
    }

    MNodeX++;


  /** Determine the subgraph isomorphism nodes **/
  /** these nodes must be newly assigned for each NetNode any time a new **/
  /** model is added **/

    if(MODES[2]==2){ //NETMATCH
      //determineNoEdgeTests();
    }

  }else{
    GRAIN=0;

    if(EndNode.NType==NNODE_TYPE){
      Token *orig;
            
      gmt_assert(NNodes[EndNode.ind].GO.count()==1);      
      orig=(Token*) NNodes[EndNode.ind].GO.remove(0);       
      //InfoNN[EndNode.ind].originalNodes.insert(orig);

     }else{
      Token *orig;
           
      gmt_assert(CNodes[EndNode.ind].GO.count()==1);      
      orig=(Token*) CNodes[EndNode.ind].GO.remove(0);       
      //InfoCN[EndNode.ind].originalNodes.insert(orig);

     }  
  }

  while(CompileBest.count()>0){
    ConnectType* CR;
    int key;
    CR=(ConnectType*) CompileBest.removeTop(&key);
    delete CR;
  }

  while(CompileBalanced.count()>0){
    ConnectType* CR;
    int key;
    CR=(ConnectType*) CompileBalanced.removeTop(&key);
    delete CR;
  }

  if(MNodeX>0)
    if(NetworkDepth<MNodes[MNodeX-1].TopNumber) NetworkDepth=MNodes[MNodeX-1].TopNumber;

  orderSuccessors();

  /* Compilation finished */

  consistencyCheck();

  if(!Late_Compile)
    clearNet();

if(DEBUGSET){
  debugDump();
}  
  
}


/************************************/
/***  compilation supporting cast  **/
/************************************/



action_element* 
NetWork::findPartial(Token* tok,ConnectType *C,List* Elist){
  action_element *P;
  int f,k,i,ni,ty;
  ConnectType *CT,*PT;

  P=new action_element;  
  
  if(MODES[1]==NORMAL){  //NET
    CompileBest.initNext();
  }else{
    CompileBalanced.initNext();
  }

  f=1;
  i=-1;

  while(f){
    if(f==1){
      ni=C->ind;
      ty=C->NType;
      f=2;
    }else{
      
      if(MODES[1]==NORMAL){ //NET
	CT=(ConnectType*) CompileBest.getnext(&k);
      }else{
	CT=(ConnectType*) CompileBalanced.getnext(&k);
      }
      
      if(CT==NULL) break;
      
      ni=CT->ind;
      ty=CT->NType;
    }
    
    P->node=ni;
    P->type=ty;

    if(ty==NNODE_TYPE){
      ACT_UID=NNodes[ni].UID;
      i=findEntry(tok,&NNodes[ni].GO,&P->tok,Elist);
    }else{
      ACT_UID=CNodes[ni].UID;
      i=findEntry(tok,&CNodes[ni].GO,&P->tok,Elist);
    }

    P->index=i;
    if(i>-1) break;

  }

  if(i==-1) {
  
    printf("FATAL: Could not find Partial!"); gmt_assert(0);
  }

  PT=new ConnectType;
  PT->ind=CNodeX;
  PT->NType=CNODE_TYPE;
  PT->direction=RIGHT;
  
  if(P->type==NNODE_TYPE){
    NNodes[P->node].right_successor.insert(PT);
    NNodes[P->node].represented++;
    if(NNodes[P->node].TMP_GO.count()>0){
	// do update later
	insertIntoUPDATE(P->node,P->type,NNodes[P->node].UID, &NNodes[P->node].TMP_GO, PT,1);
      }
  }else{
    CNodes[P->node].right_successor.insert(PT);
    CNodes[P->node].represented++;
    if(CNodes[P->node].TMP_GO.count()>0){
	// do update later
	insertIntoUPDATE(P->node,P->type,CNodes[P->node].UID, &CNodes[P->node].TMP_GO, PT,1);
      }
 }
  return P;

}



int
NetWork::findEntry(Token *tok,List* go,Token **rtok,List* Elist){
  Token *htok,*hetok;
  int i;
  
  i=0;
  go->reset();
  
  while(htok=(Token*) go->getnext()){
    
    if(!(tok->intersect(htok))){
      /** set of nodes may not intersect **/
      
      findBridges(tok,htok,Elist);
      
      if(Elist->count()>0){
	/** setof vertices must intersect **/
	
	if(MODES[1]==BALANCED)   //NET
	  if(!isUsedInBalanced(htok,ACT_UID)){
	    while(Elist->count()>0){
	      BridgeData* bd=(BridgeData*) Elist->remove(0);
	      delete bd;
	    }
	    continue;
	  }
	*rtok=htok;
	
	return i;
      }
    }
    i++;
  }
  return -1;
}




action_element* 
NetWork::createNode(action_element* P,Token* tok,List* Elist){
  int i,x,y;
  action_element* Q;
  BridgeData* bd;

  /*** Create a CNODES from the given token **/
  /*   the leftTest contains a number at a   */
  /*   position if the node is shared        */
  /*                                         */
  /***                                      **/
  
  i=CNodeX;
  
  CNodes[i].testList=(*Elist);
  
#if 0
  CNodes[i].testList.reset();
  while(bd=(BridgeData*) CNodes[i].testList.getnext()) 
    CNodes[i].totalDeletionCost+=ATTC->deletionCost(bd->id);
#endif

  CNodes[i].newLength=tok->length()+P->tok->length();
  CNodes[i].parents[1].ind=P->node;
  CNodes[i].parents[1].NType=P->type;
  CNodes[i].parents[1].direction=RIGHT;
  CNodes[i].UID=UIDLimit;
  UIDLimit++;
  CNodes[i].totalEdges=(*Elist).count();

  
  if(P->type==NNODE_TYPE){
    CNodes[i].TopNumber=NNodes[P->node].TopNumber+1;
    CNodes[i].totalEdges+=NNodes[P->node].totalEdges;
    CNodes[i].totalDeletionCost=NNodes[P->node].totalDeletionCost;
  }else{
    CNodes[i].TopNumber=CNodes[P->node].TopNumber+1;
    CNodes[i].totalEdges+=CNodes[P->node].totalEdges;
    CNodes[i].totalDeletionCost=CNodes[P->node].totalDeletionCost;
  }

#if 0
  CNodes[i].testList.reset();
  while(bd=(BridgeData*) CNodes[i].testList.getnext()) 
    CNodes[i].totalDeletionCost+=ATTC->deletionCost(bd->id);
#endif

  CNodeX++;

  Q=new action_element;
  Q->node=i;
  Q->type=CNODE_TYPE;
  
  return Q;
}




void 
NetWork::TetMarch(int node){
  int x,y,p1,p2,i,j,changes;
  action_element P;
  Token *tok1,*tok2,*etok1,*op_etok,*etok;
  Token *htok,*tok;
  List *HGO,*HEDGE;
  int l_node;
  

/*** move possible tokens from the parents          **/
/*** if BALANCED then check whether EX[i]==UID      **/
/*** only under this condition may a token be moved **/
/*** otherwise tokens, which are locked elsewhere   **/
/*** would be subjecy to early movement             **/


  P.node=node;
  P.type=CNODE_TYPE;
  P.direction=CNodes[node].parents[0].direction;

  changes=1;

  while(changes){
    changes=0;
    j=0;
    
    l_node=CNodes[node].parents[0].ind;

    if(CNodes[node].parents[0].NType==NNODE_TYPE){
      HEDGE=&NNodes[l_node].EdgeGO;
      HGO=&NNodes[l_node].GO;
      ACT_UID_P=NNodes[l_node].UID;
    } else{
      HEDGE=&CNodes[l_node].EdgeGO;
      HGO=&CNodes[l_node].GO;
      ACT_UID_P=CNodes[l_node].UID;
    }

    HGO->reset();
    HEDGE->reset();
    
    while(htok=(Token*) HGO->getnext()){
      
      P.tok=htok;
      P.etok=(Token*) HEDGE->getnext();
        
      i=0;
      while(1){
	int fl;

	P.index=i;
	fl=getTokenPair(&P,&tok1,&tok2);
	
	if(!((tok1)&&(tok2))) break;

	if(MODES[1]==BALANCED)  //NET 
	  if(fl==0){
	    i++;
	    continue;
	  }	

	tok=CTest(node,tok1,tok2);
	if(tok){
	  CNodes[node].GO.insert(tok);
	 
	  
	  /** move and remove recursively  **/

	  moveTokenToINFO(&CNodes[node].parents[0],tok1);
	  moveTokenToINFO(&CNodes[node].parents[1],tok2);
	  
	  removeFromGO(node,tok);

	  changes=1;
	  break;
	}else{
	  i++;
	}
      }
      
      if(changes) break; /* if a tok was found break the loop ad restart */
      j++;
      
  
    }/* march through the HGO list */
    
  }/** while changes are happening do it again **/
}



int
NetWork::isComplete(Token* tok){

  if(tok->length()==WModel->numberOfVertices())
    return 1;
  else
    return 0;
}




void
NetWork::moveTokenToINFO(ConnectType *C,Token *tok1){
  int x;
  Token* t;
  int node;
  int i,*m;
  

  node=C->ind;

  if(C->NType==NNODE_TYPE){
    NNodes[node].GO.reset();
    i=0;
    while(tok1!=NNodes[node].GO.getnext()) i++;
    
    t=(Token*) NNodes[node].GO.remove(i);
   
    gmt_assert(t==tok1);
    
    if(GRAIN==0){
      InfoNN[node].originalNodes.insert(tok1);
      
      
      m=new int;
      *m=WMNumber;
      InfoNN[node].originalModels.insert(m);
    }
  }else{
    CNodes[node].GO.reset();
    i=0;
    while(tok1!=CNodes[node].GO.getnext()) i++;

    t=(Token*) CNodes[node].GO.remove(i);
 
    gmt_assert(t==tok1);
    
    if(GRAIN==0){
      InfoCN[node].originalNodes.insert(tok1);
      
      m=new int;
      *m=WMNumber;
      InfoCN[node].originalModels.insert(m);
    }

    if(tok1->inparents[0]!=NULL) moveTokenToINFO(&CNodes[node].parents[0],tok1->inparents[0]);
    if(tok1->inparents[1]!=NULL) moveTokenToINFO(&CNodes[node].parents[1],tok1->inparents[1]); 
  }

}



void
NetWork::findBridges(Token* ltok,Token *rtok,List* Elist){
  int x,y,l,r;
  BridgeData *bd;
  AttId at;
  int edg;

  l=ltok->length();
  r=rtok->length();

  for(x=0;x<l;x++){
    
    for(y=0;y<r;y++){
      WModel->initNext((*ltok)[x],(*rtok)[y]);
      while((at=WModel->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	bd=new BridgeData;
	bd->n1=x;
	bd->n2=y;
	bd->id=at;
	bd->direction=1;
	Elist->insert(bd);
      }

      if(WModel->isDirected()){
	WModel->initNext((*rtok)[y],(*ltok)[x]);
	while((at=WModel->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	  bd=new BridgeData;
	  bd->n1=x;
	  bd->n2=y;
	  bd->id=at;
	  bd->direction=-1;
	  Elist->insert(bd);
	}
      }      
    }
  }
}


int 
NetWork::removeFromSTOP(Token* tok,SortedList& STOP){
  int ret,key,YES,x,y;
  Token* ctok;
  ret=0;
  STOP.initNext();
  while(ctok=(Token*) STOP.getnext(&key)){
    
    YES=0;
    for(x=0;x<tok->length();x++){
      for(y=0;y<ctok->length();y++){
	if(((*tok)[x]==-1)||((*ctok)[y]==-1)) continue;
	if((*tok)[x]==(*ctok)[y]){YES=1;break;}
// check for identities in SuperNodes
	if(WModel->inSuper((*tok)[x],(*ctok)[y])){YES=1;break;}
      }
      if(YES) break;
    }

    if(YES){

//    if(tok->intersect(ctok)){
      STOP.deleteCurrent();
      removed_instances++;
      delete ctok;
      ret=1;
    }
  }
  return ret;
}



int
NetWork::removeFromGO(Token *tok,List& GO,double &FUTerr,double &FUT){
  int ret,x,i,YES,y;
  Token* tok2;
  double err=INFTY_COST;
  ret=0;
  i=0;
  while(i<GO.count()){
    tok2=(Token*) GO.get(i);
    
// instead of using intersect (because of possible SuperNodes

    YES=0;
    for(x=0;x<tok->length();x++){
      for(y=0;y<tok2->length();y++){
	if(((*tok)[x]==-1)||((*tok2)[y]==-1)) continue;
	if((*tok)[x]==(*tok2)[y]){YES=1;break;}
// check for identities in SuperNodes
	if(WModel->inSuper((*tok)[x],(*tok2)[y])){YES=1;break;}
      }
      if(YES) break;
    }

    if(YES){
//    if(tok->intersect(tok2)){
      tok2=(Token*) GO.remove(i);
      removed_instances++;
      delete tok2;
      ret=1;
    }else{
      if(err>tok2->totalErr())
	err=tok2->totalErr();
      i++;
    }
  }
  
  if(err>FUT) FUTerr=FUT;
  else FUTerr=err;
  
  return ret;
}



/* Remove from ALL CNodes except the one nummered <node> */
void
NetWork::removeFromGO(int node,Token *tok){
  int x,i;
  Token* tok2;


  for(x=0;x<CNodeX;x++){
    if(node==x) continue;
    if(!CNodes[x].GO.count()) continue;
    
    i=0;
    while(i<CNodes[x].GO.count()){
      tok2=(Token*) CNodes[x].GO.get(i);
      if(tok->intersect(tok2)){
	tok2=(Token*) CNodes[x].GO.remove(i);
	delete tok2;
     }else{
	i++;
      }
    }
  }
 
}

void
NetWork::orderSuccessors(){

  int x,key;
  SortedList A,D;
  ConnectType *C;
  
  D.descending();

  for(x=0;x<NNodeX;x++){

    while(NNodes[x].right_successor.count()>0){
      C=(ConnectType*) NNodes[x].right_successor.remove(0);
      A.insert(C,0,CNodes[C->ind].TopNumber);
    }
    while(A.count()>0) {
      C=(ConnectType*) A.removeTop(&key);
      NNodes[x].right_successor.insert(C);
    }

   while(NNodes[x].left_successor.count()>0){
      C=(ConnectType*) NNodes[x].left_successor.remove(0);
      D.insert(C,0,CNodes[C->ind].TopNumber);
    }
    while(D.count()>0) {
      C=(ConnectType*) D.removeTop(&key);
      NNodes[x].left_successor.insert(C);
    }
  }

  for(x=0;x<CNodeX;x++){

    while(CNodes[x].right_successor.count()>0){
      C=(ConnectType*) CNodes[x].right_successor.remove(0);
      A.insert(C,0,CNodes[C->ind].TopNumber);
    }
    while(A.count()>0) {
      C=(ConnectType*) A.removeTop(&key);
      CNodes[x].right_successor.insert(C);
    }

   while(CNodes[x].left_successor.count()>0){
      C=(ConnectType*) CNodes[x].left_successor.remove(0);
      D.insert(C,0,CNodes[C->ind].TopNumber);
    }
    while(D.count()>0) {
      C=(ConnectType*) D.removeTop(&key);
      CNodes[x].left_successor.insert(C);
    }
  }
}



void
NetWork::determineNoEdgeTests(){
  int x,i,skip,*mod,y;
  Token *newt,*tok,*etok,*newt2;
  List B;
  int IdSGI;

  gmt_assert(0);

  /** clear previous lists first **/

   for(x=0;x<CNodeX;x++)
     while(CNodes[x].SGI.count()){
       tok=(Token*) CNodes[x].SGI.remove(0);
       delete tok;
     }
       


  for(x=0;x<WModel->numberOfVertices();x++) VX[x]=-1;

  IdSGI=0;
  for(x=0;x<CNodeX;x++){
    skip=0;
    InfoCN[x].originalNodes.reset();
    InfoCN[x].originalEdges.reset();
    InfoCN[x].originalModels.reset();
    while(tok=(Token*) InfoCN[x].originalNodes.getnext()){
      etok=(Token*) InfoCN[x].originalEdges.getnext();
      mod=(int*) InfoCN[x].originalModels.getnext();
      newt=new Token(CNodes[x].newLength);
      i=0;
      for(y=0;y<tok->length();y++){
	if(edgesAreEnclosed((*tok)[y],etok,*mod)){
	  newt->set(i,y);
	  i++;
	}    
      }
      if(i<=1){
	skip=1;
	delete newt;
	break;
      }else{
	newt2=new Token(i);
	for(y=0;y<i;y++) newt2->set(y,(*newt)[y]);
	B.insert(newt2);
	delete newt;
      }
    }
    if(skip) continue;
    newt=NULL;
    newt2=NULL;
    newt2=new Token(1);
    tok=(Token*) B.remove(0);
    (*newt2)=(*tok);
    delete tok;

    while(B.count()){
      tok=(Token*) B.remove(0);
      newt=(*tok)*(*newt2);
      delete newt2;
      delete tok;
      newt2=newt;
      newt=NULL;
      if(newt2==NULL) break;

    }

    if(newt2==NULL) continue;

    fillNoEdgeList(newt2,x);

    
    InfoCN[x].originalNodes.reset();
    while(tok=(Token*) InfoCN[x].originalNodes.getnext()){
      IdSGI++;      
      for(y=0;y<newt2->length();y++)
	VX[(*tok)[(*newt2)[y]]]=IdSGI;
    }
  }
}



int
NetWork::edgesAreEnclosed(int v,Token* etok,int mod){
  int x,out,in,e;
  AttId at;
  Token t(1);

  gmt_assert(0);
  return 1;
}



void
NetWork::fillNoEdgeList(Token* rtok,int i){
  int x,k,y,z;
  int *mod;
  Token* tok,*htok,*ntok;

  gmt_assert(0);

}


/************************************/
/***  Test methods                 **/
/************************************/



Token*
NetWork::NTest(int i,Token* tok){
  double er,edge_er;
  AttId at;
  AttId e_l[2];
  int dim_l=0;
  BridgeData* bd;

#ifdef STUDIES
  NNodes[i].testTry++;
#endif

  if((*tok)[0]==-1){
     er=ATTC->deletionCost(NNodes[i].TestID);
   }else{
     at=WModel->getNodeAttributeId((*tok)[0]);
     er=ATTC->error(NNodes[i].TestID,at);
   }
  edge_er=0;
     
  if(er<0){
    /** impossible substitution  **/
    return NULL;
  }


#ifdef STUDIES
  NNodes[i].testPassed++;
#endif
 
  /** must consider slings also */
 
  NNodes[i].testList.reset();
  while(bd=(BridgeData*) NNodes[i].testList.getnext())
    e_l[dim_l++]<bd->id;

#ifdef NEW_ATTOBJECT
  e_l[dim_l++].set(-1,NULL,0);
#else
  e_l[dim_l++]=-1;
#endif
  edge_er=edgeErrorBetween2Vertices(WModel,ATTC,e_l,dim_l,(*tok)[0],(*tok)[0],1);

  if(er+edge_er>cut_threshold) return NULL;
  if(er+edge_er>=INFTY_COST) return NULL;

  if(er+edge_er>0){
    Token *etok;
    
    if(mode==COMPILE_MODE) return NULL;
    
    //   if(er>ATTC->insertionCost(NNodes[i].TestID)) return NULL;

    etok=new Token;
    (*etok)=(*tok);
    etok->setErr(0,er);
    etok->setEdgeErr(edge_er);
    return etok;
    
  }else{
    Token* etok;

    etok=new Token;
    (*etok)=(*tok);
    etok->setErr(0,0);
    
    return etok;
  }
  
}





Token* 
NetWork::CTest(int n,Token* tok1,Token* tok2){

  if(!((tok1)&&(tok2))) return NULL;

  if(tok1->totalErr()+tok2->totalErr()>=cut_threshold){
    return NULL;
  }

  int x,y;
  
 /*  The subgraphs denoted by toke1 and tok2 must be disjoint */  


  if(disjoint_test_func==NULL){
    for(x=0;x<tok1->length();x++){
      for(y=0;y<tok2->length();y++){
	if(((*tok1)[x]==-1)||((*tok2)[y]==-1)) continue;
	
	if((*tok1)[x]==(*tok2)[y]) return NULL;
// check for identities in SuperNodes
	if(WModel->inSuper((*tok1)[x],(*tok2)[y])) return NULL;
      }
    }
  }else{
    for(x=0;x<tok1->length();x++){
      for(y=0;y<tok2->length();y++){
	if(((*tok1)[x]==-1)||((*tok2)[y]==-1)) continue;
	
	if(disjoint_test_func((*tok1)[x],(*tok2)[y],WModel)==0) return NULL;
      }
    }
  }

  int x0,y0,x1,y1,xi,yi;
  int l,r;
  Token *newt;
  int maps;
  double toterr;
  int tedge,l0,r0;
  BridgeData *bd;
  double terr,err;
  int edge,firsttime;
  double subgraph_iso_err;
  int dim_l,i,j;
  int dir_o_a[2];
  int n1_o,n2_o,dir_o,upper,lower;
  AttId e_l[2];

  toterr=0;

  ctest_count++;
 

  /* Settings  */

  
  CNodes[n].testList.reset();
  toterr=0;
  x1=-1;  y1=-1;
  subgraph_iso_err=0;
  firsttime=1;
  dim_l=0;
  n1_o=0;
  n2_o=-1;
  if(WModel->isDirected()){
    dir_o_a[0]=0;
    dir_o_a[1]=0;
  }else{
    dir_o_a[0]=1;
    dir_o_a[1]=1;
  }
  int Loop=1;

  BRIDGE_DUMMY.n1=tok1->length()-1;
  BRIDGE_DUMMY.n2=tok2->length();
 
  while(Loop){
   
    if(toterr>cut_threshold) return NULL;
    if(toterr>=INFTY_COST) return NULL;

    bd=(BridgeData*) CNodes[n].testList.getnext();
   
    if((bd)&&(bd->direction==dir_o)&&(bd->n1==n1_o)&&(bd->n2==n2_o)){
      e_l[dim_l++]<bd->id;
    }else{
      if(!firsttime){
#ifdef NEW_ATTOBJECT
	e_l[dim_l++].set(-1,NULL,0);
#else
	e_l[dim_l++]=-1;
#endif	
	toterr+=edgeErrorBetween2Vertices(WModel,ATTC,e_l,dim_l,(*tok1)[n1_o],(*tok2)[n2_o],dir_o);
	
      } /* firsttime */    
      
      if(toterr>cut_threshold) return NULL;

      if(bd==NULL){
	Loop=0;
	bd=&BRIDGE_DUMMY;
      }

     if((MODES[3]>1)||(mode==COMPILE_MODE)){

/* delete edges which appear in between */
/* but only if subgraph or isomorphim is asked for */

#ifdef NEW_ATTOBJECT
	e_l[0].set(-1,NULL,0);
#else
	e_l[0]=-1;
#endif
	dim_l=1;
	for(i=n1_o;i<=bd->n1;i++){
	  if(i==n1_o)
	    lower=n2_o+1;
	  else
	    lower=0;
	  if(i==bd->n1)
	    upper=bd->n2;
	  else
	    upper=tok2->length();
	  
	  for(j=lower;j<upper;j++){
	    toterr+=edgeErrorBetween2Vertices(WModel,ATTC,e_l,dim_l,(*tok1)[i],(*tok2)[j],1);
	    if(WModel->isDirected())
	      toterr+=edgeErrorBetween2Vertices(WModel,ATTC,e_l,dim_l,(*tok1)[i],(*tok2)[j],-1);
	    
	    
	    if(toterr>cut_threshold) return NULL;
	  }
	}
      }
/* Are there edges in both direction in the original model? */
/* If not, check image for extraneous edges                 */

      if(!firsttime){
	if((n1_o!=bd->n1)||(n2_o!=bd->n2)){
	  dim_l=1;
	  e_l[0]=-1;
	  if(dir_o_a[0]==0) 
	    toterr+=edgeErrorBetween2Vertices(WModel,ATTC,e_l,dim_l,(*tok1)[n1_o],(*tok2)[n2_o],-1);
	  if(dir_o_a[1]==0)
	    toterr+=edgeErrorBetween2Vertices(WModel,ATTC,e_l,dim_l,(*tok1)[n1_o],(*tok2)[n2_o],1);
	  if(WModel->isDirected()){
	    dir_o_a[0]=0;
	    dir_o_a[0]=0;
	  }
	}
      }
      
/* start a new edge calculation */
      
      if(Loop){
	firsttime=0;
	n1_o=bd->n1;
	n2_o=bd->n2;
	dir_o=bd->direction;
	dir_o_a[(dir_o+1)/2]=1;
	dim_l=0;
	e_l[dim_l++]=bd->id;
      }
    }
  }

#ifdef CONTROL_TEST
  if(toterr>=INFTY_COST) {
    Edit_Desc *eds;
    while(SUB_list.count()>0){
      eds=(Edit_Desc*) SUB_list.remove(0);   
      
      delete eds;
    }
    
    while(DEL_list.count()>0){
      eds=(Edit_Desc*) DEL_list.remove(0);
      
      delete eds;
    }
    
    while(INS_list.count()>0){
      eds=(Edit_Desc*) INS_list.remove(0); 
      
      delete eds;
    }  
    return NULL;
  }
#else
  if(toterr>=INFTY_COST) return NULL;
#endif

  if(toterr>cut_threshold) return NULL;

  if(toterr>0)
    if(mode==COMPILE_MODE) return 0;
     
  newt=new Token(CNodes[n].newLength);
  
  for(x=0;x<tok1->length();x++){
    newt->set(x,(*tok1)[x]);
    newt->setErr(x,tok1->err(x));
  }

  x=tok1->length();
  for(y=0;y<tok2->length();y++){
    newt->set(y+x,(*tok2)[y]);
    newt->setErr(y+x,tok2->err(y));
  }

  newt->setEdgeErr(toterr+tok1->edgeErr()+tok2->edgeErr());
  
#ifdef CONTROL_TEST
  Token *aa1,*aa2;
  int ddd,ii;
  Edit_Desc* eds;
  ii=0;
  ddd=SUB_list.count()+DEL_list.count()+INS_list.count();
  aa2=new Token(ddd);
  aa1=NULL;
  while(SUB_list.count()>0){
    eds=(Edit_Desc*) SUB_list.remove(0);   
    aa2->set(ii,n);
    aa2->setErr(ii++,eds->err);
    delete eds;
  }
    
  while(DEL_list.count()>0){
    eds=(Edit_Desc*) DEL_list.remove(0);
    aa2->set(ii,n);
    aa2->setErr(ii++,eds->err);
    delete eds;
  }

  while(INS_list.count()>0){
    eds=(Edit_Desc*) INS_list.remove(0); 
    aa2->set(ii,n);
    aa2->setErr(ii++,eds->err);
    delete eds;
  }  

  aa1=NULL;

  if(tok1->dim==1) tok1->inparents[0]=NULL;
  if(tok2->dim==1) tok2->inparents[0]=NULL;

  if((tok1->inparents[0]!=NULL)&&(tok2->inparents[0]!=NULL)){
    aa1=(*tok1->inparents[0])+(*tok2->inparents[0]);
  }else{
    if(tok1->inparents[0]!=NULL)
      aa1=tok1->inparents[0];
    if(tok2->inparents[0]!=NULL)
      aa1=tok2->inparents[0];
  }

  
  if((aa1!=NULL)&&(aa2!=NULL)){
    newt->inparents[0]=(*aa1)+(*aa2);
  }else{
    if(aa1!=NULL)
      newt->inparents[0]=aa1;
    if(aa2!=NULL) 
      newt->inparents[0]=aa2;
  }

#endif

  if(newt->totalErr()>cut_threshold){
    delete newt;
    return NULL;
  }

  return newt;
      
}





void
NetWork::SGItest(Token* ntok,int i){
  int x,ed;
  Token* tok;
  AttId at;
  double sc;
  int l,r;

  gmt_assert(0);

}



/***********************************/
/*    RETE  control methods        */
/***********************************/


void
NetWork::run(int v){
  action_element *P;
  int x;
  Token *tok;

  tok=new Token(1);
  tok->set(0,v);
  Token::inst_count--;

/** Prepare the stack with node entries **/

  for(x=0;x<NNodeX;x++){
   
    P=new action_element;
    P->node=x;
    P->type=NNODE_TYPE;
    P->tok=tok;
    P->index=0;
    ACTION.push(TREAT,P);
  }

/** start RETE **/

  process();

  delete tok;

}



void
NetWork::process(){
  action_element *P;
  int key;  
  int stopWhile=0;

  while(!ACTION.empty()){

    if(POLLINGSET){
      polling();
    }

    P=(action_element*) ACTION.pop(&key);
    
    switch(key){
      
    case SAVE: saveIn(P);
      
      if(DEBUGSET){
	debug(P->node,P->type,P->tok,3);
      }	    
      delete P;
      break;
      
    case TREAT:
      if(DEBUGSET){
	debug(P->node,P->type,P->tok,0);
      }
      treatIn(P);
      break;
      

    case EXIT: 
      stopWhile=1;
      break;
    }

    if(stopWhile) break;

  }
}


void
NetWork::REMOVErun(int* vertices,int n){
  int x;
  gmt_assert(attributeInit);

  cout << "Removing Instances .... ";
  
  REMOVE_MODE=1;
  removed_instances=0;
  for(x=0;x<n;x++){
    if(DEBUGSET){
      debugDump();
    }
    run(vertices[x]);
    Hidden.insert(x);
  }

  cout << removed_instances << " deleted\n";
  REMOVE_MODE=0;
  
  if(MODES[5]>1){
    cout << "Looking ahead after removal of instances...";
    LECRFutureError(2);
    LECRassignFUTerr();
    LECRresortOPEN();
    cout << "  done\n";
  }
}


void 
NetWork::continueProcess(AdjazenzMatrix *M){
  gmt_assert(attributeInit);

  setMode(DEBUG_MODE);
  setData(M);
  if(MODES[3]==1) ATTC->setInsertionCost(0);
  process();

}


int
NetWork::runSpecified(Token* tok, AdjazenzMatrix *M){
  int x,d;
  int SUCCESS,key;

  gmt_assert(attributeInit);

  setMode(DEBUG_MODE);
  setData(M);
  
  if(MODES[3]==1) ATTC->setInsertionCost(0);

  CurrentBestTry.clear();
  
  startTime=elapsed_time();
  
  gmt_assert(ACTION.empty());
  
  d=WModel->numberOfVertices();
  
  cout << "Exact  RETE with specified list ....\n";
  
  for(x=0;x<tok->length();x++){
    if(DEBUGSET){
      debugDump();
    }
    run((*tok)[x]);
  }
 
  
  if(DEBUGSET){
    debugDump();
  }
  
  SUCCESS=Collection.count();
  
  TotalCollection+=Collection; 

  if(OPEN.count()>0) return 2;
  else return 1;
}





int
NetWork::SECRrun(AdjazenzMatrix *M,double threshold){
  int x,d;
  int SUCCESS,key;

  gmt_assert(attributeInit);
  
  clearNet();

  setMode(DEBUG_MODE);
  setData(M);
  
  STORE_IN_STOP=0;

  if(MODES[2]==2) ENV_CONTROL_STACK=1; 
  if(MODES[3]==1) ATTC->setInsertionCost(0);

  OPEN.clear();
  CurrentBestTry.clear();
  
  startTime=elapsed_time();
  
  gmt_assert(ACTION.empty());
  
  d=WModel->numberOfVertices();
  
  cout << "Exact RETE ....\n";
  
  for(x=0;x<d;x++){
    if(DEBUGSET){
      debugDump();
    }
    run(x);
  }
 
  
  if(DEBUGSET){
    debugDump();
  }
  
  SUCCESS=Collection.count();
  

  if(MODES[4]==1){ 
// only if inexact versions are looked for **/
    
   
    /** Fill NNodes with inserted Node token specially marked with node=-1**/
   
   
    run(-1);


    if(MODES[5]>1){
      LECRFutureError(1);
      LECRassignFUTerr();
      if(DEBUGSET){
	debugDump();
      }
      
      LECRresortOPEN();
    }
    
  }
  
  if(ENV_CONTROL_STACK)
    prepareOPENandSTACK();

  TotalCollection+=Collection;


  if((!SUCCESS)) SECRcontinue(threshold);
  ATTC->setInsertionCost(1);

  if(MODES[2]==2) ENV_CONTROL_STACK=0; 

  STORE_IN_STOP=0;

  if(OPEN.count()>0) return 2;
  else return 1;
}



int
NetWork::SECRcontinue(double threshold){
  action_element* P;
  int key;    
  int x,d,SUCCESS;
  double BestValue;

  if(MODES[3]==1) ATTC->setInsertionCost(0);  
  if(MODES[2]==2) ENV_CONTROL_STACK=1; 

  STORE_IN_STOP=0;

  startTime=elapsed_time();
  
  /* Move the previously found instances */

  gmt_assert(ACTION.empty());


  Collection.clear();

  /**************************************************/
  /** Move always cheapest token until termination **/
  /** This is the heart piece  of SECR, its main   **/
  /** difference to normal RETE                    **/
  /**************************************************/
    
  cout << "SECR procedure...\n";
  
  if(threshold<0)
    BestValue=30000000;
  else
    BestValue=threshold;
  
  SUCCESS=0;


  /*******************************************************/
  /* if RETURN_ON_FIND is true, then we return everytime */
  /* a new instance has been found, otherwise we search  */
  /* until the error of the instances exceeds the error  */
  /* of the best found instances in this loop OR         */
  /* if no instance was found, until the general limit   */
  /* is exceed                                           */
  /* If threshold==-1 and RETURN_ON_FIND == 0            */
  /* then search for all best instances and equal costs  */

  /* if threshold==-1 and RETURN_ON_FIND == 1            */
  /* then search for next best instance and return       */

  /* if threshold>=0 and RETURN_ON_FIND == 0             */
  /* then search for all instances with cost < threshold */

  /* if threshold>=0 and RETURN_ON_FIND == 1             */
  /* then search for first instance with cost < threshold */
  /*******************************************************/

  
  while((OPEN.count())&&(!SUCCESS)){      
    double currVal;
    
    if(STOP_BIT) {STOP_BIT=0;break;}
    
    currVal=OPEN.TopValue();
    
    if(currVal>BestValue+FPepsilon){
      SUCCESS=1;
      continue;
    }
    
    P=(action_element*) OPEN.removeTop(&key);
    
    switch(P->type){
    case NNODE_TYPE:
      ruleMove(P->node,P->type,P->tok,NULL,&NNodes[P->node].left_successor,&NNodes[P->node].right_successor,&NNodes[P->node].models);
      NNodes[P->node].STOP.removeTop(&key);
      if(NNodes[P->node].STOP.count()>0){
	insertIntoOPEN(P->node,P->type,NNodes[P->node].UID,(Token*) NNodes[P->node].STOP.top(&key));
      }
      break;
      
    case CNODE_TYPE:
     
      if(MODES[5]==5){
	if(renewEstimation(P)){
	  Token* stok=(Token*) CNodes[P->node].STOP2.removeTop(&key);
	  if(CNodes[P->node].STOP2.count()>0){
	    insertIntoOPEN(P->node,P->type,CNodes[P->node].UID,(Token*) CNodes[P->node].STOP2.top(&key));
	  }
	  gmt_assert(CNodes[P->node].STOP.removeElement(stok)!=NULL);
	}else{
	  OPEN.insert(P,key,P->tok->totalErr());
	  continue;
	}
      }else{
	CNodes[P->node].STOP.removeTop(&key);
	if(CNodes[P->node].STOP.count()>0){
	  insertIntoOPEN(P->node,P->type,CNodes[P->node].UID,(Token*) CNodes[P->node].STOP.top(&key));
	}
      }

      ruleMove(P->node,P->type,P->tok,NULL,&CNodes[P->node].left_successor,&CNodes[P->node].right_successor,&CNodes[P->node].models);
      
      break;
      
    case MNODE_TYPE: 
      saveIn(P);
      MNodes[P->node].STOP.removeTop(&key);
      if(MNodes[P->node].STOP.count()>0)
	insertIntoOPEN(P->node,P->type,MNodes[P->node].UID,(Token*) MNodes[P->node].STOP.top(&key));
      break;
    } 
    
    process();


    /******* Determine if node is terminal *****/

    if(TERMINAL_DETECTION){
      
      switch(P->type){
      case NNODE_TYPE:
	if(NNodes[P->node].STOP.count()==0){
	  NNodes[P->node].terminal=1;
	  NNodes[P->node].FUT=INFTY_COST+1;
	  LECRstartDynamicEstimate(P);
	}
	break;
	
      case CNODE_TYPE:
	if(CNodes[P->node].STOP.count()==0){     
	  if(CNodes[P->node].parents[0].NType==NNODE_TYPE)
	    CNodes[P->node].terminal=NNodes[CNodes[P->node].parents[0].ind].terminal;
	  else
	    CNodes[P->node].terminal=CNodes[CNodes[P->node].parents[0].ind].terminal;
	  
	  if(CNodes[P->node].parents[1].NType==NNODE_TYPE)
	    CNodes[P->node].terminal*=NNodes[CNodes[P->node].parents[1].ind].terminal;
	  else
	    CNodes[P->node].terminal*=CNodes[CNodes[P->node].parents[1].ind].terminal;
	  
	  if(CNodes[P->node].terminal)
	    LECRstartDynamicEstimate(P);
	}
	break;
      }
    }
    
    if(MODES[5]>1){
      if((OPEN.TopValue()-currVal>=HEURISTIC1)){ 
	
#if 0      
	LECRFutureError(2);
	LECRassignFUTerr();
	LECRresortOPEN();
#else
	action_element *EP,*P2;
	
	ERROR_ESTIMATION=1;
	UPDATE_ESTIMATION=1;
	EP=new action_element;
	P2=new action_element;
	*P2=*P;
	P2->tok=DUMMY_TOKEN;
	ACTION.push(EXIT,EP);
	treatIn(P2);   
	process();
	delete EP;
	ERROR_ESTIMATION=0;
	UPDATE_ESTIMATION=0;
	
#endif 
      }
    }
    
    
    /**************************************************/
    /** depending on whether a monomorphism, subgraph **/
    /** isomorphism or Isomorphism was calculated in **/
    /** Model node, BestValue may be higher than the **/
    /** net value in the network                     **/
    /**************************************************/
    
    delete P;

    if(Collection.count()) {
      BestValue=Collection.TopValue();
      if(RETURN_ON_FIND)
	SUCCESS=1;
    }
   
    
    
    if(ENV_CONTROL_STACK)
      if(CONTROL_GO->count()>0)
	prepareOPEN();
  } 
  
  endTime=elapsed_time();
  
  if(DEBUGSET){
    debugDump();
  }
    
#ifdef DEBUGTREE
    displayTreeAscii();
#endif
    
  ATTC->setInsertionCost(1);
    
  TotalCollection+=Collection;

  if(MODES[2]==2) ENV_CONTROL_STACK=0; 

  STORE_IN_STOP=0;

  if(OPEN.count()>0) return 2;
  else return 1;
}




void 
NetWork::LECRFutureError(int t){
  int x,y,i;

  ERROR_ESTIMATION=t;
  
//  i=Token::inst_count;
  run(0);
//  Token::inst_count=i;
  
  ERROR_ESTIMATION=0;

}



void
NetWork::LECRresortOPEN(){
  int x;
  SortedList OPEN2;
  action_element* Q;
  double err,i_err;
  int key;

  while(OPEN.count()>0){
    err=OPEN.TopValue();
    Q=(action_element*) OPEN.removeTop(&key);
    
    switch(Q->type){
    case NNODE_TYPE:
      err=NNodes[Q->node].STOP.TopValue();
      err+=NNodes[Q->node].assignedFUTerr;
      OPEN2.insert(Q,key,err);
      break;
   
    case CNODE_TYPE:
      if(MODES[5]==5){
	Token* tok;

#if 0
	CNodes[Q->node].STOP2.initNext();
	while(tok=(Token*) CNodes[Q->node].STOP2.getnext(&key)){
	  i_err=tok->totalErr();
	  tok->calcError();
	  i_err=i_err-tok->totalErr();
	  if(i_err<CNodes[Q->node].assignedFUTerr)
	    i_err=CNodes[Q->node].assignedFUTerr;
	  tok->tot_err+=i_err;
	  temp.insert(tok,0, tok->totalErr());
	}

	
	CNodes[Q->node].STOP2=temp;
#endif

	err=CNodes[Q->node].STOP2.TopValue();
	OPEN2.insert(Q,key,err);

      }else{
	Token* tok;
	err=CNodes[Q->node].assignedFUTerr;
// 	if(MODES[5]==4){
// 	  tok=(Token*) CNodes[Q->node].STOP.top(&key);
// 	  i_err=individualLookahead(tok,Q->node,Q->type);
// 	  tok->tot_err-=i_err;
// 
// 	  if(err<i_err)
// 	    err=i_err;
// 
// 	}
	
	err+=CNodes[Q->node].STOP.TopValue();
	OPEN2.insert(Q,key,err);
      }
      break;

    case MNODE_TYPE:
      /* possible assignment of an anticipated future error */
      /* for isomorphism should be made here   */
      OPEN2.insert(Q,key,err);
      break;
    }
  }

  OPEN=OPEN2;

}


void
NetWork::LECRupdateFUT(action_element* P,double ferr,ConnectType* CP){
  ConnectType* CT;

  /** assignFutErrorrec(ConnectType* ..) therefore we must **/
  /**  transform actionelement into connectType            **/

  CT=new ConnectType;
  CT->ind=P->node;
  CT->NType=P->type;
  

  if(CP->NType==NNODE_TYPE){
    UPDATE_UID=NNodes[CP->ind].UID;
    //CT->direction=NNodes[CT->ind].UID;
  }else if(CP->NType==CNODE_TYPE){
    UPDATE_UID=CNodes[CP->ind].UID;
    //CT->direction=CNodes[CT->ind].UID;
  }

  CT->direction=-1;
  LECRassignFUTrec(CT,ferr);
  delete CT;
}


void
NetWork::LECRstartDynamicEstimate(action_element* P){
  Token* tok;
  action_element *EP;

  // WATCH THE BEHAVIOR OF THIS!
  // return;
  // DOESN'T Do ANYTHING!

  ERROR_ESTIMATION=1;
  UPDATE_ESTIMATION=1;

  EP=new action_element;
  ACTION.push(EXIT,EP);

  switch(P->type){
  case NNODE_TYPE:
    tok=NNodes[P->node].FutErrorToken;
     ruleMove(P->node,P->type,tok,NULL,&NNodes[P->node].left_successor,&NNodes[P->node].right_successor,&NNodes[P->node].models);
  break;

  case CNODE_TYPE:
    tok=CNodes[P->node].FutErrorToken;
     ruleMove(P->node,P->type,tok,NULL,&CNodes[P->node].left_successor,&CNodes[P->node].right_successor,&CNodes[P->node].models);
    break;
  }
   
  process();
  delete EP;

  ERROR_ESTIMATION=0;
  UPDATE_ESTIMATION=0;
  
}



Token*
NetWork::LECRErrorToken(double fut_l,double fut_r,double err_l,double err_r,Token* tok_l,Token* tok_r,int node,int type){
  int x,FUT;
  double FUTerr,v,di;
  Token *ret;
  double result;


  if((tok_l==NULL)||(tok_r==NULL)) return NULL;

  switch(type){

  
     
  case CNODE_TYPE:

#if 1

    double ff1,ff2,ff0;

    ff1=fut_l+err_r;
    ff2=fut_r+err_l;

    if(ff1>ff2) ff1=ff2;
    if(CNodes[node].STOP.count()>0){
      ff2=CNodes[node].STOP.TopValue();
      if(ff1>ff2) ff1=ff2;
    }

    ff0=ff1;

// if GO not empty, then FUTerr reflects the true error so far
    if(CNodes[node].GO.count()>0)
      ff0=CNodes[node].FUTerr;

    int fl=0;
    if((CNodes[node].FUTerr<ff0)||(CNodes[node].FUT<ff1))
      fl=1;

    CNodes[node].FUTerr=ff0;
    CNodes[node].FUT=ff1;
    
    if(ENV_SECOND_HEURISTIC)
      averageFUTerr(&CNodes[node],NULL);

    if((UPDATE_ESTIMATION)||(ERROR_ESTIMATION==3)){
      if(fl) return DUMMY_TOKEN;
      else return NULL;
    }
    return DUMMY_TOKEN;


#else

    if(UPDATE_ESTIMATION){
      double t_err=0;

      if((fut_l*err_l)>(fut_r*err_r)){
	t_err=err_l;
	t_err+=fut_r*err_r;
	
      }

      if((fut_l*err_l)<=(fut_r*err_r)){
	t_err=err_r;
	t_err+=fut_l*err_l;
      }


      if((CNodes[node].terminal)&&(CNodes[node].GO.count()==0))
	 t_err=INFTY_COST+1;

      if(t_err>CNodes[node].FUTerr){
	if(DEBUGSET)
	  cout << "old Error:" << CNodes[node].FUTerr << "  new Error:" << t_err << "\n";
	CNodes[node].FUTerr=t_err;
	
	return CNodes[node].FutErrorToken;
      }else{
	return NULL;
      }
      
    }else{
      if(CNodes[node].GO.count()>0) CNodes[node].FUT=0;
      else CNodes[node].FUT=1;
      v=CNodes[node].STOP.TopValue();

      ret=DUMMY_TOKEN;
      
      di=0;

      if((fut_l==0)&&(fut_r==0)){
	int x;
	if(err_l<err_r) result=err_l;
	else result=err_r;
	if(CNodes[node].STOP.count()>0){
	  if(result>CNodes[node].STOP.TopValue())
	    result=CNodes[node].STOP.TopValue();
	}
	CNodes[node].FUTerr=result;	
      }

      
      if((fut_l==1)&&(fut_r==0)){
	int x;
	CNodes[node].FUTerr=err_l;
      }
      
      if((fut_l==0)&&(fut_r==1)){
	int x,y;
	di=0;
	CNodes[node].FUTerr=err_r;
      }
      
      
      if((fut_l==1)&&(fut_r==1)){
	int x,y;
	di=0;
	FUTerr=err_l+err_r;
	CNodes[node].FUTerr=FUTerr;
      }
    }
#endif
    break;
  }
 
  return ret;

}



void
NetWork::LECRassignFUTerr(){
  int x;
  ConnectType* CP,C;

  for(x=0;x<MNodeX;x++){ 
    MNodes[x].parents.reset();
    while(CP=(ConnectType*) MNodes[x].parents.getnext()){
      C=*CP;
      C.direction=0;
      LECRassignFUTrec(&C,0);
    }
  }
}



void
NetWork::LECRassignFUTrec(ConnectType *C,double err){
  int x,key,k;
  double ferr;
  ConnectType* Q;
  double oldAss;

/*********************************************************/
/* if FUTlist ([2,a],[3,b]) and [4,a] is inserted        */
/* then ([3,b],[4,a]) will give a new assigned error     */
/* of 3  (4 was sent). therefore new_err=3 and 3 is sent */
/* to the parents as the new assignedFUTerr of this node */
/* and not 4 (this was a BUG !!)                         */
/*********************************************************/

  double new_err;   

  k=C->direction;

  switch(C->NType){

  case NNODE_TYPE:

    if(UPDATE_ESTIMATION){
      gmt_assert(C->direction>-1);
      //if(NNodes[C->ind].UID==UPDATE_UID) return;

      
      if(DEBUGSET){
	cout << "old Assigned:" << NNodes[C->ind].assignedFUTerr;
	cout << " new Assigned:" << err << "\n";
      }

/* Keep list of successors */

      if(DEBUGSET){
	if(NNodes[C->ind].FUTlist.count()>1) cout << "Multiple Successors updated\n";
      }

      Q=(ConnectType*) NNodes[C->ind].FUTlist.removeKey(k);
      if(Q==0) Q=new ConnectType;
  
 
      oldAss=NNodes[C->ind].assignedFUTerr;     
      NNodes[C->ind].FUTlist.insert(Q,k,err);
      NNodes[C->ind].assignedFUTerr=NNodes[C->ind].FUTlist.TopValue();

#if 0
      assert(oldAss-FPepsilon<=NNodes[C->ind].assignedFUTerr);
#else
      if(!(oldAss-FPepsilon<=NNodes[C->ind].assignedFUTerr))
	fputs("WARNING: new assigned error lower than previous!",stderr);
#endif

      if(fabs(oldAss-NNodes[C->ind].assignedFUTerr)<=FPepsilon) return;

      if(NNodes[C->ind].STOP.count()>0){
	insertIntoOPEN(C->ind,C->NType,NNodes[C->ind].UID,(Token*) NNodes[C->ind].STOP.top(&key));
	  
      }
	
      
    }else{
      Q=(ConnectType*) NNodes[C->ind].FUTlist.removeKey(k);
      
      if(Q==0) Q=new ConnectType;
      
      NNodes[C->ind].FUTlist.insert(Q,k,err);
      NNodes[C->ind].assignedFUTerr=NNodes[C->ind].FUTlist.TopValue();
   
    }

    break;


  case CNODE_TYPE:

    if(UPDATE_ESTIMATION){
      //if(CNodes[C->ind].UID==UPDATE_UID) return;

      if(C->direction>-1){
	if(DEBUGSET){
	  cout << "old Assigned:" << CNodes[C->ind].assignedFUTerr;
	  cout << " new Assigned:" << err << "\n";
	}
	
	if(DEBUGSET){
	  if(CNodes[C->ind].FUTlist.count()>1) cout << "Multiple Successors updated\n";
	}
	
	Q=(ConnectType*) CNodes[C->ind].FUTlist.removeKey(k);
	if(Q==0)   Q=new ConnectType;
	
      
	oldAss=CNodes[C->ind].assignedFUTerr;
	CNodes[C->ind].FUTlist.insert(Q,k,err);
	CNodes[C->ind].assignedFUTerr=CNodes[C->ind].FUTlist.TopValue();

	if(!(oldAss-FPepsilon<=CNodes[C->ind].assignedFUTerr))
	  fputs("WARNING: new assigned error lower than previous!",stderr);
  

	if(fabs(oldAss-CNodes[C->ind].assignedFUTerr)<=FPepsilon) return;
	

	if(CNodes[C->ind].STOP.count()>0){
	  insertIntoOPEN(C->ind,C->NType,CNodes[C->ind].UID,(Token*) CNodes[C->ind].STOP.top(&key));
	}
      }
    }else{      
      Q=(ConnectType*) CNodes[C->ind].FUTlist.removeKey(k);
      if(Q==0) Q=new ConnectType;
      
      
      oldAss=CNodes[C->ind].assignedFUTerr;
      CNodes[C->ind].FUTlist.insert(Q,k,err);
      CNodes[C->ind].assignedFUTerr=CNodes[C->ind].FUTlist.TopValue();
      
     
      //if(fabs(oldAss-CNodes[C->ind].assignedFUTerr)<=FPepsilon) return;

    }

    new_err=CNodes[C->ind].assignedFUTerr;
    
    ferr=0;
    if(CNodes[C->ind].parents[1].NType==NNODE_TYPE){
    
	ferr=NNodes[CNodes[C->ind].parents[1].ind].FUTerr;
    }else{
    
	ferr=CNodes[CNodes[C->ind].parents[1].ind].FUTerr;
    }


    double ferr2;
    if(ENV_SECOND_HEURISTIC){
      ferr2=getSecondHeuristic(CNodes[C->ind].parents[1],CNodes[C->ind].parents[0]);
      if(ferr2>ferr); ferr=ferr2;
    }

    if(ferr<0) ferr=0;

    ferr+=new_err;

    ConnectType CO;
    CO=CNodes[C->ind].parents[0];
    CO.direction=CNodes[C->ind].UID;
    LECRassignFUTrec(&CO,ferr);

    ferr=0;
    if(CNodes[C->ind].parents[0].NType==NNODE_TYPE){
      ferr=NNodes[CNodes[C->ind].parents[0].ind].FUTerr;
    }else{

      ferr=CNodes[CNodes[C->ind].parents[0].ind].FUTerr;
    }

    if(ENV_SECOND_HEURISTIC){
      ferr2=getSecondHeuristic(CNodes[C->ind].parents[0],CNodes[C->ind].parents[1]);
      if(ferr2>ferr); ferr=ferr2;
    }

    if(ferr<0) ferr=0;
    ferr+=new_err;

    
    CO=CNodes[C->ind].parents[1];
    CO.direction=CNodes[C->ind].UID;
    LECRassignFUTrec(&CO,ferr);

    
    break;
  }
}




int
NetWork::getTokenPair(action_element* P,Token** tok1,Token** tok2){
  int i;
  
  gmt_assert((P->direction==LEFT)||(P->direction==RIGHT));

  i=P->index;
  if(P->direction==LEFT)
    {*tok1=P->tok;

     if(CNodes[P->node].parents[1].NType==NNODE_TYPE){
       ACT_UID=NNodes[CNodes[P->node].parents[1].ind].UID;
        
       *tok2=(Token*) NNodes[CNodes[P->node].parents[1].ind].GO.get(i);
     }else{
       ACT_UID=CNodes[CNodes[P->node].parents[1].ind].UID;      
       *tok2=(Token*) CNodes[CNodes[P->node].parents[1].ind].GO.get(i);
     }

     if((MODES[1]==BALANCED)&&(mode==COMPILE_MODE)){ //NET
       if(!isUsedInBalanced(*tok1,ACT_UID_P)||
	  (!isUsedInBalanced(*tok2,ACT_UID))){
	 return 0;
       }
     }
   }
     
  if(P->direction==RIGHT)
    {*tok2=P->tok;
  
     if(CNodes[P->node].parents[0].NType==NNODE_TYPE){
       ACT_UID=NNodes[CNodes[P->node].parents[0].ind].UID;
      
       *tok1=(Token*) NNodes[CNodes[P->node].parents[0].ind].GO.get(i);
     }else{
       ACT_UID=CNodes[CNodes[P->node].parents[0].ind].UID;
      
       *tok1=(Token*) CNodes[CNodes[P->node].parents[0].ind].GO.get(i);
     }

     if((MODES[1]==BALANCED)&&(mode==COMPILE_MODE)){ //NET
       if(!isUsedInBalanced(*tok2,ACT_UID_P)||
	  (!isUsedInBalanced(*tok1,ACT_UID))){
	 return 0;
       }
     }
   }
  
  return 1;
}



void
NetWork::saveIn(action_element *P){
  ConnectType *C;
  int do_dynamic=0;

  if(REMOVE_MODE) return;
 

  switch(P->type){

  case NNODE_TYPE: 
    if(ERROR_ESTIMATION){
      NNodes[P->node].FutErrorToken=P->tok;

    }else{
      if(MODES[5]>1){
	
	gmt_assert((NNodes[P->node].FUTerr-FPepsilon)<=P->tok->totalErr());
	
	do_dynamic=0;
	if(ENV_SECOND_HEURISTIC)
	  do_dynamic=averageFUTerr(&NNodes[P->node],P->tok);

	if(NNodes[P->node].GO.count()==0){
	  if(DEBUGSET)
	    cout << "Diff. Error-Estimated:" << P->tok->totalErr()-NNodes[P->node].FUTerr << "\n";
	  
	  if((P->tok->totalErr()-NNodes[P->node].FUTerr>0)){
	    NNodes[P->node].FUTerr=P->tok->totalErr();
	    do_dynamic=1;
	  }
	}	
	if(do_dynamic)
	  LECRstartDynamicEstimate(P);
	 
	
      }

      if(mode==COMPILE_MODE) {
	VX[(*P->tok)[0]]=P->node;
	
	if(!CompileBest.isKey(NNodes[P->node].UID)){
	  C=new ConnectType;
	  C->NType=NNODE_TYPE;
	  C->ind=P->node;
	  CompileBest.insert(C,NNodes[P->node].UID,NNodes[P->node].TopNumber);
	}
      }

      
      NNodes[P->node].GO.insert(P->tok);

    }
    break;
      
    
  case CNODE_TYPE:
    if(ERROR_ESTIMATION){

      CNodes[P->node].FutErrorToken=P->tok;

    }else{

      if(MODES[5]>1){


	do_dynamic=0;
	if(ENV_SECOND_HEURISTIC)
	  do_dynamic=averageFUTerr(&CNodes[P->node],P->tok);
	
// 	if(MODES[5]==4){
// 	  
// 	  if((CNodes[P->node].FUTerr>P->tok->totalErr())||(CNodes[P->node].GO.count()==0)){
// 	    do_dynamic=1;
// 	    CNodes[P->node].FUTerr=P->tok->totalErr();
// 	    if(CNodes[P->node].FUTerr>CNodes[P->node].FUT)
// 	      CNodes[P->node].FUTerr=CNodes[P->node].FUT;
// 	  }
// 	}else{
	
	gmt_assert((CNodes[P->node].FUTerr-FPepsilon)<=P->tok->totalErr());	  
	


	if(MODES[5]<5){
	  if(CNodes[P->node].GO.count()==0){
	    if(DEBUGSET)
	      cout << "Diff. Error-Estimated:" << P->tok->totalErr()-CNodes[P->node].FUTerr;
	    
	    if((P->tok->totalErr()-CNodes[P->node].FUTerr>0)){
	   
	      CNodes[P->node].FUTerr=P->tok->totalErr();
	      
	      do_dynamic=1;
	      
	    }
	  }
	}else{
	  if(MODES[5]==5){
	    double err;
	    if(CNodes[P->node].GO.count()==0){
	      if(P->tok->totalErr()<CNodes[P->node].FUT)
		err=P->tok->totalErr();
	      else
		err=CNodes[P->node].FUT;
	      
	      if(err>CNodes[P->node].FUTerr){
		CNodes[P->node].FUTerr=err;
		do_dynamic=1;
	      }
	    }
	  }
	}
	if(do_dynamic)
	  LECRstartDynamicEstimate(P);
      }
      
      
#ifdef DEBUG      
      if(CNodes[P->node].GO.count()==0)
	enterCurrentBestTry(P->node,P->tok);
#endif      

      CNodes[P->node].GO.insert(P->tok);
      if(mode==COMPILE_MODE){
	if(!CompileBest.isKey(CNodes[P->node].UID)){
	  C=new ConnectType;
	  C->NType=CNODE_TYPE;
	  C->ind=P->node;

//	  CompileBest.insert(C,CNodes[P->node].UID,CNodes[P->node].TopNumber);
	  CompileBest.insert(C,CNodes[P->node].UID,CNodes[P->node].newLength);
	}
      }
    }
    
    break;

    
  case MNODE_TYPE:
    {
      MappingData *MP;
      Token* tok;
      double d;

      MP= new MappingData;
      MP->setName(MNodes[P->node].modelName);
      MP->setNumber(MNodes[P->node].modelNr);
      MP->setMatch(P->tok,&MNodes[P->node].Original);
      tok=new Token();
      (*tok)=(*P->tok);

      d=0;       

      MNodes[P->node].instances.insert(tok);
      Collection.insert(MP,MNodes[P->node].modelNr,tok->totalErr());

      if(Estimation_Series!=NULL){
	ConnectType* CP;
	CP=(ConnectType*) MNodes[P->node].parents.get(0);

	Estimation_Series->next=new Series_Type;
	Estimation_Series= Estimation_Series->next;
	Estimation_Series->err=-(P->tok->totalErr());
	Estimation_Series->next=NULL;
	//Estimation_Series->time=Timex->time_stamp();
	Estimation_Series->time = elapsed_time();
     
      }

      if(RETURN_ON_FIND)
	STORE_IN_STOP=1;   // Stop processing and wait till found instances are removed!
    }
  }
}


void
NetWork::treatIn(action_element *P){

  Token *tok,*tok1,*tok2;
  Token *op_etok,*etok1;
  int edgeNr,k;

  switch(P->type){
    int i;
    
  case NNODE_TYPE:  
    if(ERROR_ESTIMATION){
    
      double est_err;
      if(NNodes[P->node].STOP.count()>0){
	est_err=NNodes[P->node].STOP.TopValue();
  
	NNodes[P->node].FUT=est_err;
      }else{
	NNodes[P->node].FUT=INFTY_COST+1;
      }

      tok=DUMMY_TOKEN;
#if 1
      if(NNodes[P->node].GO.count()==0)
	NNodes[P->node].FUTerr=NNodes[P->node].FUT;
      
#else
      if(NNodes[P->node].GO.count()>0) NNodes[P->node].FUT=0;
      else NNodes[P->node].FUT=1;
      
      NNodes[P->node].FUTerr=est_err;
#endif  

      ruleMove(P->node,P->type,tok,NULL,&NNodes[P->node].left_successor,&NNodes[P->node].right_successor,&NNodes[P->node].models);
      
      
    }else{
      if(REMOVE_MODE){
	Token* htop=(Token*) NNodes[P->node].STOP.top(&k);
	if(removeFromSTOP(P->tok, NNodes[P->node].STOP)){
	  // has top of STOP changed ?
	  if(htop!=(Token*) NNodes[P->node].STOP.top(&k))
	    insertIntoOPEN(P->node,P->type,NNodes[P->node].UID,(Token*) NNodes[P->node].STOP.top(&k));
	}
	
	if(removeFromGO(P->tok,NNodes[P->node].GO,NNodes[P->node].FUTerr,NNodes[P->node].FUT)){
	  ruleMove(P->node,P->type,P->tok,NULL,&NNodes[P->node].left_successor,&NNodes[P->node].right_successor,&NNodes[P->node].models);
	}  
	
	delete P;
	break;
      }
      
     

      if(tok=NTest(P->node,P->tok)){
	if(tok==NULL) break;
	if((tok->totalErr()>0)||(STORE_IN_STOP)){
	  if(((mode==COMPILE_MODE)||(MODES[4]==2))&&(tok->totalErr()>0)){
	    delete tok;
	    Token::inst_count--;
	    delete P;
	    break;
	  }
	  
	  NNodes[P->node].STOP.insert(tok, 0,tok->totalErr());
	 
	  if(tok==(Token*) NNodes[P->node].STOP.top(&k))
	    insertIntoOPEN(P->node,P->type,NNodes[P->node].UID,tok);
	  
	}else{
	  ruleMove(P->node,P->type,tok,NULL,&NNodes[P->node].left_successor,&NNodes[P->node].right_successor,&NNodes[P->node].models);
	}  
      }
    }
    delete P;
    break;
    
    
  case CNODE_TYPE:
    if(ERROR_ESTIMATION){
      int &term=CNodes[P->node].terminal;
      if(CNodes[P->node].STOP.count()==0)
	term=1;
      else
	term=0;

      if((P->direction==LEFT)){
	if(CNodes[P->node].parents[0].NType==NNODE_TYPE){
	  if(CNodes[P->node].parents[1].NType==NNODE_TYPE){
	    term*=NNodes[CNodes[P->node].parents[0].ind].terminal*NNodes[CNodes[P->node].parents[1].ind].terminal;
	    
	    tok=LECRErrorToken(NNodes[CNodes[P->node].parents[0].ind].FUT,
			       NNodes[CNodes[P->node].parents[1].ind].FUT,
			       NNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       NNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       P->tok,
			       NNodes[CNodes[P->node].parents[1].ind].FutErrorToken,
			       P->node,
			       P->type);
	  }else{
	    term*=NNodes[CNodes[P->node].parents[0].ind].terminal*CNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(NNodes[CNodes[P->node].parents[0].ind].FUT,
			       CNodes[CNodes[P->node].parents[1].ind].FUT,
			       NNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       CNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       P->tok,
			       CNodes[CNodes[P->node].parents[1].ind].FutErrorToken,
			       P->node,
			       P->type);
	  }
	}else{
	  if(CNodes[P->node].parents[1].NType==NNODE_TYPE){
	    term*=CNodes[CNodes[P->node].parents[0].ind].terminal*NNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(CNodes[CNodes[P->node].parents[0].ind].FUT,
			       NNodes[CNodes[P->node].parents[1].ind].FUT,
			       CNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       NNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       P->tok,
			       NNodes[CNodes[P->node].parents[1].ind].FutErrorToken,
			       P->node,
			       P->type);
	  }else{
	    term*=CNodes[CNodes[P->node].parents[0].ind].terminal*CNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(CNodes[CNodes[P->node].parents[0].ind].FUT,
			       CNodes[CNodes[P->node].parents[1].ind].FUT,
			       CNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       CNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       P->tok,CNodes[CNodes[P->node].parents[1].ind].FutErrorToken,
			       P->node,
			       P->type);
	  }
	}

	if(UPDATE_ESTIMATION) LECRupdateFUT(P,CNodes[P->node].assignedFUTerr,&CNodes[P->node].parents[0]);

      }else{
	if(CNodes[P->node].parents[0].NType==NNODE_TYPE){
	  if(CNodes[P->node].parents[1].NType==NNODE_TYPE){
	    term*=NNodes[CNodes[P->node].parents[0].ind].terminal*NNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(NNodes[CNodes[P->node].parents[0].ind].FUT,
			       NNodes[CNodes[P->node].parents[1].ind].FUT,
			       NNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       NNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       NNodes[CNodes[P->node].parents[0].ind].FutErrorToken,
			       P->tok,
			       P->node,
			       P->type);
	  }else{
	    term*=NNodes[CNodes[P->node].parents[0].ind].terminal*CNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(NNodes[CNodes[P->node].parents[0].ind].FUT,
			       CNodes[CNodes[P->node].parents[1].ind].FUT,
			       NNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       CNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       NNodes[CNodes[P->node].parents[0].ind].FutErrorToken,
			       P->tok,
			       P->node,
			       P->type);
	  }
	}else{
	  if(CNodes[P->node].parents[1].NType==NNODE_TYPE){
	    term*=CNodes[CNodes[P->node].parents[0].ind].terminal*NNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(CNodes[CNodes[P->node].parents[0].ind].FUT,
			       NNodes[CNodes[P->node].parents[1].ind].FUT,
			       CNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       NNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       CNodes[CNodes[P->node].parents[0].ind].FutErrorToken,
			       P->tok,
			       P->node,
			       P->type);
	  }else{
	    term*=CNodes[CNodes[P->node].parents[0].ind].terminal*CNodes[CNodes[P->node].parents[1].ind].terminal;

	    tok=LECRErrorToken(CNodes[CNodes[P->node].parents[0].ind].FUT,
			       CNodes[CNodes[P->node].parents[1].ind].FUT,
			       CNodes[CNodes[P->node].parents[0].ind].FUTerr,
			       CNodes[CNodes[P->node].parents[1].ind].FUTerr,
			       CNodes[CNodes[P->node].parents[0].ind].FutErrorToken,
			       P->tok,
			       P->node,
			       P->type);
	  }
	}

       if(UPDATE_ESTIMATION) LECRupdateFUT(P,CNodes[P->node].assignedFUTerr,&CNodes[P->node].parents[1]);

      }
      
      if(tok)
	ruleMove(P->node,P->type,tok,NULL,&CNodes[P->node].left_successor,&CNodes[P->node].right_successor,&CNodes[P->node].models);

      delete P;
    }else{
      if(REMOVE_MODE){
	Token* htop=(Token*) CNodes[P->node].STOP.top(&k);
	if(removeFromSTOP(P->tok,CNodes[P->node].STOP)){
	  // has top of STOP changed ?
	   if(htop!=(Token*) CNodes[P->node].STOP.top(&k))
	    insertIntoOPEN(P->node,P->type,CNodes[P->node].UID,(Token*) CNodes[P->node].STOP.top(&k));
	 }

	if(removeFromGO(P->tok,CNodes[P->node].GO,CNodes[P->node].FUTerr,CNodes[P->node].FUT)){
	   ruleMove(P->node,P->type,P->tok,NULL,&CNodes[P->node].left_successor,&CNodes[P->node].right_successor,&CNodes[P->node].models);
	}  
	
	delete P;
	break;
      }

      getTokenPair(P,&tok1,&tok2);
      
      if(!((tok1)&&(tok2))){ delete P; break;}
      
      P->index++;
      ACTION.push(TREAT,P);
      
      if(tok=CTest(P->node,tok1,tok2)){
	if(tok==NULL) break;
	double i_err=0;

	if((MODES[5]==0)||(MODES[5]==5)){ // same lookahea as A*
	  
	  i_err=individualLookahead(tok,P->node,P->type);
	  
	  if(MODES[5]==0) i_err=0;
	  else if(MODES[5]==5){
	    if(CNodes[P->node].FutErrorToken!=NULL){
	      if(i_err<CNodes[P->node].assignedFUTerr){ 
		tok->tot_err-=i_err;
		i_err=CNodes[P->node].assignedFUTerr;
		tok->tot_err+=i_err;
	      }
	    }
	  }
//	  assert(tok->totalErr()>=tok1->totalErr());
//	  assert(tok->totalErr()>=tok2->totalErr());

	  // these corrections are allowed, because the future error caluclation
	  // may decline slighlty

	  if(tok->totalErr()<tok1->totalErr())
	    tok->tot_err=tok1->tot_err;

	  if(tok->totalErr()<tok2->totalErr())
	    tok->tot_err=tok2->tot_err;


	}

	if((tok->totalErr()>0)||(STORE_IN_STOP)){
	  if(((mode==COMPILE_MODE)||(MODES[4]==2))&&(tok->totalErr()>0)){
	    delete tok;
	    Token::inst_count--;
	    break;
	  }


	  double err_level=0;

	  if(CNodes[P->node].FutErrorToken!=NULL){
	    err_level=tok->totalErr()+CNodes[P->node].assignedFUTerr;
	  }

	  if(MODES[5]==4){
	    i_err=individualLookahead(tok,P->node,P->type);
	    
	    if(err_level<tok->totalErr())
	      err_level=tok->totalErr();
	    tok->tot_err-=i_err;
	    i_err=0;
	  }
	  
	    
	  if(err_level<cut_threshold){
	    
	    
	    CNodes[P->node].STOP.insert(tok, 0,tok->totalErr()-i_err);
	    
	    if(MODES[5]==5){
	      CNodes[P->node].STOP2.insert(tok, 0,tok->totalErr());
	      if(tok==(Token*) CNodes[P->node].STOP2.top(&k))
		insertIntoOPEN(P->node,P->type,CNodes[P->node].UID,tok);
	      
	    }else{
	      if(tok==(Token*) CNodes[P->node].STOP.top(&k))
		insertIntoOPEN(P->node,P->type,CNodes[P->node].UID,tok);
	    } 
	  }
	}else{	
	  Token *etok;
	  etok=NULL;
	  
	  if(mode==COMPILE_MODE){
	    	    
	    tok->inparents[0]=tok1;
	    tok->inparents[1]=tok2;
	    
	  }
	  ruleMove(P->node,P->type,tok,NULL,&CNodes[P->node].left_successor,&CNodes[P->node].right_successor,&CNodes[P->node].models);
	}  
      }
    }
      
    break;

  case MNODE_TYPE:  /** Model found **/
    if(ERROR_ESTIMATION){
      ConnectType* CP;
      if(DEBUGSET){
	cout << "Error Estimation for Model " << MNodes[P->node].modelNr;
	CP=(ConnectType*) MNodes[P->node].parents.get(0);
	if(CP->NType==CNODE_TYPE){
	  cout << CNodes[CP->ind].FUT*CNodes[CP->ind].FUTerr << "\n";
	}
      }
      if(Estimation_Series!=NULL){
	CP=(ConnectType*) MNodes[P->node].parents.get(0);
	if(CNodes[CP->ind].FUTerr>Estimation_Series->err){
	  Estimation_Series->next=new Series_Type;
	  Estimation_Series= Estimation_Series->next;
	  Estimation_Series->err=CNodes[CP->ind].FUTerr;
	  Estimation_Series->next=NULL;
	  //Estimation_Series->time=Timex->time_stamp();
	  Estimation_Series->time = elapsed_time();
	}  
      }
      
    }else{
      MappingData *MP;
      Token* tok;
      double d;
      
      
      if((MODES[3]==3)||(mode==COMPILE_MODE)){
	if((MODES[4]>2)||(mode==COMPILE_MODE)){ //exact isomorphism
	  // compare the number of vertices in the model and the input
	  if(MNodes[P->node].Original.length()==WModel->numberOfVertices())
	    saveIn(P);
	}else{
	  tok=new Token();
	  (*tok)=(*P->tok);
	  tok->addEdgeErr(isomorphismError(P->tok));
	  MNodes[P->node].STOP.insert(tok, 0,tok->totalErr());
	  if(tok==(Token*) MNodes[P->node].STOP.top(&k))
	    insertIntoOPEN(P->node,P->type,MNodes[P->node].UID,tok);
	}
      }else{
	saveIn(P);
      }
    }
  

   delete P;
   break;
   
  }
}



double
NetWork::subgraphError(Token* tok,int i){
  int x,y,g,l,r,edg;
  Token* et;
  int m;
  double er,d;
  AttId at,wat;
  
  gmt_assert(0);

  return d;
}


double
NetWork::isomorphismError(Token* tok){
  double d,di;
  AttId at;
  int e,x,out,in;


if(DEBUGSET){
  cout << "\nSubError: " << d;
}

  for(x=0;x<WModel->numberOfVertices();x++) VX[x]=1;
  for(x=0;x<WModel->numberOfEdges();x++) EX[x]=1;

  for(x=0;x<tok->length();x++) VX[(*tok)[x]]=0;
  
  di=0;

  for(x=0;x<WModel->numberOfVertices();x++){
    if(VX[x]){
      WModel->initNext(x);
      while((e=WModel->getnext(&out,&in,&at))!=-1){
	if(EX[e]==1){
	  EX[e]=0;
	  di+=ATTC->insertionCostOfEdge(at,1-2*VX[out],1-2*VX[in],NULL);  // 1-2*x=1 if x=0 ; =-1 if x=1
	}
      }
      VX[x]=0;
      at=WModel->getNodeAttributeId(x);

      di+=ATTC->insertionCost(at);
    }
  }

  return di;

}




void
NetWork::insertIntoOPEN(int node,int type,int uid,Token* tok){
  int x;
  action_element *P,*Q;
  double err;

  Q=(action_element*) OPEN.removeKey(uid);
  
  // element is removed from STOP, now return if tok==NULL
  if(tok==NULL) {
    if(Q!=NULL) delete Q;
    return;
  }  
  
  if(Q==NULL){
    Q=new action_element;
    Q->node=node;
    Q->type=type;
  }

  Q->tok=tok;
  
  double i_err=0;
  err=0;
  if((MODES[5]>1)&&(MODES[5]<5)){

    switch(type){
    case NNODE_TYPE: 
      if(NNodes[node].FutErrorToken!=NULL)
	err=NNodes[node].assignedFUTerr; 
      break;
      
    case CNODE_TYPE: 
      if(CNodes[node].FutErrorToken!=NULL)
	err+=CNodes[node].assignedFUTerr; 
      
      if(i_err>err) err=i_err;
      break;
      
    }
  }
  
  err+=Q->tok->totalErr();
  
  if((err>cut_threshold)||(err>INFTY_COST)) return;

  if(type==CNODE_TYPE){
    if(HEURISTIC2>0)
      err=err*(1-CNodes[node].newLength*HEURISTIC2);
  }  

  if(ENV_CONTROL_STACK){
    if(type==NNODE_TYPE){
      if(!ACTIVE.in(NNodes[node].UID)) return;
    }else{
      if(!ACTIVE.in(CNodes[node].UID)) return;
    }
  }
 
  OPEN.insert(Q,uid,err);
  
}


void
NetWork::insertIntoUPDATE(int node,int type,int uid,List* tmp_go,ConnectType* CT,int dir){
  update_element* Q;
  Q=(update_element*) UPDATE_LIST.getKeyData(uid);
  
  if(Q==NULL){
    Q=new update_element;
    Q->node=node;
    Q->type=type;
    Q->tmp_go=tmp_go;    
    UPDATE_LIST.insert(Q,uid,uid);
  }
  
  if(dir==0)
    Q->left.insert(CT);
  else
    Q->right.insert(CT);

}


void
NetWork::topologicalSort(){};


void
NetWork::clearNet(){
  int x,key;
  MappingData* mp;
  int *t;
  action_element *P;

  while(OPEN.count()>0){
    P=(action_element*) OPEN.removeTop(&key);
    delete P;
  }

  for(x=0;x<NNodeX;x++)
    NNodes[x].clear();


  for(x=0;x<CNodeX;x++)
    CNodes[x].clear();

  for(x=0;x<MNodeX;x++)
    MNodes[x].clear();


  if(VX!=NULL) delete VX;
  if(EX!=NULL) delete EX;

  VX=NULL;
  EX=NULL;

 

  Collection.clear();

  while(TotalCollection.count()>0){
    mp=(MappingData*) TotalCollection.removeTop(&key);
    delete mp;
  }
 
  TotalCollection.clear();  

  while(CurrentBestTry.count()>0){
    t=(int*) CurrentBestTry.removeTop(&key);
    delete t;
  }

  CurrentBestTry.clear();

  ERROR_ESTIMATION=0;
  UPDATE_ESTIMATION=0;


  Hidden.clear();
  
  ctest_count=0;

  STORE_IN_STOP=0;

#ifdef STUDIES
  Token::inst_count=0;
  Token::sumOfLength=0;
  Token::maxError=0;
  for(x=0;x<StatLevelCount;x++) Token::StatLevel[x]=0;
#endif
}


void
NetWork::discard(){
  int x;
  
#if 1
  for(x=0;x<NNodeX;x++){
    NNodes[x].discard();
    InfoNN[x].discard();
  }   



  for(x=0;x<CNodeX;x++){
    CNodes[x].discard();
    InfoCN[x].discard();
  }

  for(x=0;x<MNodeX;x++){
    MNodes[x].discard();
    InfoMN[x].discard();
  }

#endif

  if(NNodeX) delete[] NNodes;
  if(CNodeX) delete[] CNodes;
  if(MNodeX) delete[] MNodes;

  if(InfoNN) delete[] InfoNN;
  if(InfoCN) delete[] InfoCN;
  if(InfoMN) delete[] InfoMN;


  NNodes=NULL;
  CNodes=NULL;
  MNodes=NULL;
  InfoNN=NULL;
  InfoCN=NULL;
  InfoMN=NULL;



  NNodeX=0;
  MNodeX=0;
  CNodeX=0;
  NetworkDepth=0;
 
}

/********************************************/
/* allow the query of the network contents  */
/* return partial matchings along with      */
/* the models that fit                      */
/********************************************/

int 
NetWork::initQuery(int argument){
  int x,y,k;
  ConnectType *C;
  // argument may be 1=#vertices, 2=#edges 3=#v+e or 4=costs for subgraph

  while(QueryList.count()>0){
    C=(ConnectType*) QueryList.removeTop(&k);
    delete C;
  }

  if(argument>4) return 0;

  for(x=0;x<NNodeX;x++){
    if(NNodes[x].GO.count()>0){
 
      C=new ConnectType;
      C->ind=x;
      C->NType=1;
      
      switch(argument){
      case 1: QueryList.insert(C,x,1);
	break;
      case 2: QueryList.insert(C,x,0);
	break;
      case 3: QueryList.insert(C,x,1);
	break;
      case 4: QueryList.insert(C,x,ATTC->deletionCost(NNodes[x].TestID));
	break;
      }
    }
  }

  for(x=0;x<CNodeX;x++){
    if(CNodes[x].GO.count()>0){
      ConnectType *C;
      C=new ConnectType;
      C->ind=x;
      C->NType=3;
      switch(argument){
      case 1: QueryList.insert(C,x,CNodes[x].newLength);
	break;
      case 2: QueryList.insert(C,x,CNodes[x].totalEdges);
	break;
      case 3: QueryList.insert(C,x,CNodes[x].newLength+CNodes[x].totalEdges);
	break;
      case 4: 
	QueryList.insert(C,x,CNodes[x].totalDeletionCost);
      }
    }
  }
  QueryList.initNext();
  return QueryList.count();
}


int
NetWork::getPartialMatchings(List* MD_list, double *size){
  int ret;
  ConnectType* C;
  
  int key;
  MappingData *Mdata;
   Token* otok,*itok;
    int *m;
  SortedList Instances;

/*************************************************/
/* output the best matchings for these part-     */
/* ial graphs. They must be extended to the full */
/* models: 1) sort the instances 2) find the     */
/* models for each instance 3) fill in the blanks*/ 
/* for all vertices not used 4) output these     */
/* matchings                                     */
/*************************************************/

 
  C=(ConnectType*) QueryList.getnext(&key,size);

  if(!C) return 0;

  if(C->NType==NNODE_TYPE){
// Do not answer queries for single nodes

  }else{
   
    CNodes[C->ind].GO.reset();
    
    while(itok=(Token*) CNodes[C->ind].GO.getnext())
      Instances.insert(itok,0,itok->totalErr());
    
    Instances.initNext();

    while(itok=(Token*) Instances.getnext(&key)){
      InfoCN[C->ind].originalNodes.reset();
      InfoCN[C->ind].originalModels.reset();
      while(otok=(Token*) InfoCN[C->ind].originalNodes.getnext()){	
	
	m=(int*) InfoCN[C->ind].originalModels.getnext();
	Mdata=new MappingData;
	Mdata->setMatch(itok,otok);
	Mdata->setNumber(*m);
	MD_list->insert(Mdata);
      }
    }
  }
  return MD_list->count();
}



int
NetWork::NumberOfNewMatches(){
  return Collection.count();
}




MappingData*
NetWork::queryNew(int c){
  int x;

  return (MappingData*) Collection.get(c);

}

int
NetWork::NumberOfMatches(){
  return TotalCollection.count();
}


MappingData*
NetWork::query(int c){
  int x;

  return (MappingData*) TotalCollection.get(c);

}

  
void
NetWork::enclosingNodes(){}
  

void
NetWork::debug(){

  STOPPEDBYUSER=1;

  debug(0,3,NULL,0);

  STOPPEDBYUSER=0;

}





void
NetWork::ruleMove(int node,int type,Token* tok,Token* etok, List* left,List* right,List *models){
  action_element *P;
  ConnectType *ct;
  int *ci;

  while((ct=(ConnectType*) left->getnext())!=NULL){
    P=new action_element;
    P->node=ct->ind;
    P->type=ct->NType;
    P->tok=tok;
    P->index=0;
    P->direction=ct->direction;
    P->etok=etok;
    
    ACTION.push(TREAT,P);
  }


  P=new action_element;
  P->node=node;
  P->type=type;
  P->tok=tok;
  P->index=0;
  P->etok=etok;


  ACTION.push(SAVE,P);


  while((ct=(ConnectType*) right->getnext())!=NULL){
    P=new action_element;
    P->node=ct->ind;
    P->type=ct->NType;
    P->tok=tok;
    P->etok=etok;
    P->index=0;
    P->direction=ct->direction;

    ACTION.push(TREAT,P);
  }

/** push models onto stack **/

  if(models==NULL) return;

   while((ci=(int*) models->getnext())!=NULL){

    P=new action_element;
    P->node=*ci;
    P->type=MNODE_TYPE;
    P->tok=tok;
    P->etok=etok;
    P->index=0;
    P->direction=0;

    ACTION.push(TREAT,P);
  }
  
}






/***************** ALLOC NODES STORAGE ************/


void 
NetWork::reallocNodes(){
  reallocNodes(WModel->numberOfVertices(),WModel->numberOfEdges(),1);
}



void
NetWork::reallocNodes(int vertices,int edges,int modelnum){
  int x,d,NNodeDimOld;
  NNodeType *NNodes2;
  NetInfoType *InfoNN2;
  

  
  CNodeType *CNodes2;
  NetInfoType *InfoCN2;
  
  MNodeType *MNodes2;
  NetInfoType *InfoMN2;
  
  
 // assert(mode==COMPILE_MODE);
  
  
  NNodeDim=NNodeX;
  CNodeDim=CNodeX;
  MNodeDim=MNodeX;
  
  NNodeDimOld=NNodeDim;
  
  if(NNodeDim>0){
    NNodes2=NNodes;
    InfoNN2=InfoNN;
    CNodes2=CNodes;
    InfoCN2=InfoCN;
    MNodes2=MNodes;
    InfoMN2=InfoMN;
  }
  
  NNodes=new NNodeType[NNodeDim+vertices];
  InfoNN=new NetInfoType[NNodeDim+vertices];
  for(x=0;x<NNodeDim;x++){
    NNodes[x]=NNodes2[x];
    InfoNN[x]=InfoNN2[x];
  }
  NNodeDim=NNodeDim+vertices;
  
  CNodes=new CNodeType[CNodeDim+edges];
  InfoCN=new NetInfoType[CNodeDim+edges];
  for(x=0;x<CNodeDim;x++){
    CNodes[x]=CNodes2[x];
    InfoCN[x]=InfoCN2[x];
  }
  CNodeDim=CNodeDim+edges;
  
  MNodes=new MNodeType[MNodeDim+modelnum];
  InfoMN=new NetInfoType[MNodeDim+modelnum];
  for(x=0;x<MNodeDim;x++){MNodes[x]=MNodes2[x];InfoMN[x]=InfoMN2[x];};
  MNodeDim=MNodeDim+modelnum;
  
#if 1
  if(NNodeDimOld>0){
    delete[] NNodes2;
    delete[] InfoNN2;
    delete[] CNodes2;
    delete[] InfoCN2;
    delete[] MNodes2;
    delete[] InfoMN2;
  }
#endif  
  
}



void
NetWork::debugDump(){
  FILE* file;
  file=fopen("DUMP0","w");
  dump(file);
  fclose(file);
}
  


void
NetWork::dump(FILE* file){
  
  int x,y,z;
  char out[4048];

  strcpy(out,".............NNODES....................\n");
  fputs(out,file);
  
  for(x=0;x<NNodeX;x++){
    InfoNN[x].dump(file);
    NNodes[x].dump(file);
  }
  

  
  strcpy(out,".............CNODES....................\n");
  fputs(out,file);
  
  for(x=0;x<CNodeX;x++) {
    InfoCN[x].dump(file);
    CNodes[x].dump(file);


    
  }

}



/****** debugging method ********/


void
NetWork::debug(int node,int type,Token *arg,int state){
  int x,answer;


  if(!STOPPEDBYUSER){

    if(mode!=DEBUG_MODE) return;

  
    if(BREAKPOINT_SET==1)
      if((BREAKPOINT.ind!=node)||(BREAKPOINT.NType!=type)) return;

  }


  while(answer!=0){

    
    cout << "\n0)next 1)Arg 2)GO 3)STOP 4)Tests 5)Originals 6)set 7)Best 8)Dump\n";
    cin  >> answer;
    
    if(answer==6) {
      setDebug();
    }
    
    if(answer==7){
      int m,*T;
      
      CurrentBestTry.initNext();
      
      while(T=(int*) CurrentBestTry.getnext(&m)){
	Token *tok;
	if(CNodes[*T].GO.count()==0) continue;
	tok=(Token*) CNodes[*T].GO.get(0);
	cout << "Model :" << m << "  depth:" << CNodes[*T].TopNumber;
	cout << "Error :" << tok->totalErr() << "\n";
      }
      
    }
    
    
    if(answer==8){
      debugDump();
    }
    

    if(answer<6){
      
      switch(type){
	
      case NNODE_TYPE:
	cout << "NNode: " << node << " State: " << state;

	if(answer>0) display(answer,&NNodes[node].GO,&NNodes[node].STOP,&InfoNN[node].originalNodes,arg,NULL,NULL);
	
	
	break;
    
      case CNODE_TYPE: 
	cout << "CNode: " << node << " State: " << state;
	
	if(answer==6) setDebug();
#if 0	
	if(answer>0) display(answer,&CNodes[node].GO,&CNodes[node].STOP,&InfoCN[node].originalNodes,arg,&CNodes[node].leftTest,&CNodes[node].rightTest);
#endif	
	break;

      }
    }
  }

}


void
NetWork::enterCurrentBestTry(int i,Token* tok){
  int* m,*T;
  int v;
  
  InfoCN[i].originalModels.reset();

  while(m=(int*) InfoCN[i].originalModels.getnext()){

    if(CurrentBestTry.isKey(*m)){
      T=(int*) CurrentBestTry.removeKey(*m);
      if(CNodes[*T].TopNumber<CNodes[i].TopNumber){
	v=CNodes[i].TopNumber;
	*T=i;	
      }else{
	v=CNodes[*T].TopNumber;	
      }
    }else{
      T=new int;
      *T=i;
      v=CNodes[i].TopNumber;
    }
    
    CurrentBestTry.insert(T,(*m),v);
  }
}



void 
NetWork::setDebug(){
  int ans,type,ind;

  while(ans!=0){
    
    cout << "---[ 0)next   1)Node   2)BreakPoint  ]---\n";
    
    cin >> ans;
    
    cout << "\n";
    switch(ans){
    case 1: 
      cout << "Type:";
      cin >> type;
      cout << "  Node: ";
      cin >> ind;
      
      debug(ind,type,NULL,0);
      ans=0;
      break;
    case 2:
      cout << " Break Type:";
      cin >> type;
      cout << "  Break Node:";
      cin >> ind;
      
      if(type==0){ 
	BREAKPOINT_SET=0;
	break;
      }
      
      BREAKPOINT.ind=ind;
      BREAKPOINT.NType=type;
      BREAKPOINT_SET=1;
      break;
    }
  } 
}


void 
NetWork::display(int ans,List* GO,SortedList* STOP, List* orig, Token* arg,Token* ltest,Token* rtest){
  int x,y,k;
  Token* tok;
  
  cout << "\n";
  
  switch(ans){
    
  case 1: 

    if(arg!=NULL){
      cout << "(";
      for(y=0;y<arg->length();y++){
	if(y>0) cout << ",";
        cout << (*arg)[y];
      }      
    
      cout << ") ";
    }
    break;
    
  case 2:
    GO->reset();
    for(x=0;x<GO->count();x++){
      tok=(Token*) GO->getnext();
      cout << "(";
      for(y=0;y<tok->length();y++){
	if(y>0) cout << ",";
        cout << (*tok)[y];
      }      
    }
    cout << ") ";
    break;
    
    
  case 3:
    
    STOP->initNext();
    for(x=0;x<STOP->count();x++){
      tok=(Token*) STOP->getnext(&k);
      cout << "(";
      for(y=0;y<tok->length();y++){
	if(y>0) cout << ",";
	cout << (*tok)[y];
      }
    }
    cout << ")";
    break;
    
  case 4:
    cout << "(";
    for(y=0;y<ltest->length();y++){
      if(y>0) cout << ",";
      cout << (*ltest)[y];
    }
    
    cout << ") -  ";
    cout << "(";
    for(y=0;y<rtest->length();y++){
      if(y>0) cout << ",";
      cout << (*rtest)[y];
    }
    cout << ")";

    break;
    
 case 5:

    orig->reset();
    for(x=0;x<orig->count();x++){
      tok=(Token*) orig->getnext();
      cout << "(";
      for(y=0;y<tok->length();y++){
	if(y>0) cout << ",";
        cout << (*tok)[y];
      }      
    }
    cout << ") ";
    break;

    

  }

  cout << "\n";

}



void 
NetWork::getStatistics(int *expansion,int* memory,long* time,int* instances,double* minError, int* checks){
  int sumTry,sumPassed,averageTry,averagePassed,sumSteps;
  
  *expansion=Token::inst_count;
  *memory=Token::sumOfLength;
  *time=endTime-startTime;
  *instances=Collection.count();
  *minError=Collection.TopValue();
  *checks=ctest_count;

}


void
NetWork::getNodeInfo(int type,int ind,
		     int &open_nr,int& closed_nr,double& FE,double& minimal,double& nextbest,double& second_heuristic){


  if(type==NNODE_TYPE){
    open_nr=NNodes[ind].STOP.count();
    closed_nr=NNodes[ind].GO.count();
    FE=NNodes[ind].assignedFUTerr;
    minimal=NNodes[ind].FUTerr;
    nextbest=NNodes[ind].FUT;
    second_heuristic=NNodes[ind].STOP.TopValue();
//second_heuristic=NNodes[ind].FUTerr_average;
  
  }else{
    open_nr=CNodes[ind].STOP.count();
    closed_nr=CNodes[ind].GO.count();
    FE=CNodes[ind].assignedFUTerr;
    minimal=CNodes[ind].FUTerr;
    nextbest=CNodes[ind].FUT;
    second_heuristic=CNodes[ind].STOP.TopValue();
    //second_heuristic=CNodes[ind].FUTerr_average;
  }
}


void
NetWork::getAncestors(int type,int node,Hash& ht){


  if(type==NNODE_TYPE){
    if(!ht.in(NNodes[node].UID))
      ht.insert(NULL,NNodes[node].UID);
  }else{
    if(!ht.in(CNodes[node].UID))
      ht.insert(NULL,CNodes[node].UID);
    getAncestors(CNodes[node].parents[0].NType,CNodes[node].parents[0].ind,ht);
    getAncestors(CNodes[node].parents[1].NType,CNodes[node].parents[1].ind,ht);
  }
}

List*
NetWork::getNetworkData(){
  List *data,*level;
  Hash ht;
  GeneralInfoStruct* gni;
  Token *tok;
  int i,x,y,*m,*v;
  ConnectType *C;
  int *Cc;

  data= new List;

  for(x=0;x<NNodeX;x++){
    i=NNodes[x].TopNumber;
    if(!(level=(List*) ht.getKey(i))){
      level=new List;
      ht.insert(level,i);
    }

    gni=new GeneralInfoStruct;
    
    m=(int*) InfoNN[x].originalModels.get(0);
    gni->key=*m;
    gni->UID=NNodes[x].UID;
    tok=(Token*) InfoNN[x].originalNodes.get(0);
    for(y=0;y<tok->length();y++){
      v=new int;
      *v=(*tok)[y];
      gni->vertices.insert(v);
    }
    NNodes[x].left_successor.reset();
    while(C=(ConnectType*) NNodes[x].left_successor.getnext()){
      Cc=new int;
      *Cc=-CNodes[C->ind].UID;
      gni->successors.insert(Cc);
    }
    NNodes[x].right_successor.reset();
    while(C=(ConnectType*) NNodes[x].right_successor.getnext()){
      Cc=new int;
      *Cc=CNodes[C->ind].UID;
      gni->successors.insert(Cc);
    }

    gni->node=x;
    gni->type=1;
    level->insert(gni);
  }

  for(x=0;x<CNodeX;x++){
    i=CNodes[x].TopNumber;
    if(!(level=(List*) ht.getKey(i))){
      level=new List;
      ht.insert(level,i);
    }
    gni=new GeneralInfoStruct;
    
    m=(int*) InfoCN[x].originalModels.get(0);
    gni->key=*m;
    gni->UID=CNodes[x].UID;
    tok=(Token*) InfoCN[x].originalNodes.get(0);
    for(y=0;y<tok->length();y++){
      v=new int;
      *v=(*tok)[y];
      gni->vertices.insert(v);
    }

    CNodes[x].left_successor.reset();
    while(C=(ConnectType*) CNodes[x].left_successor.getnext()){
      Cc=new int;
      *Cc=-CNodes[C->ind].UID;
      gni->successors.insert(Cc);
    }
    CNodes[x].right_successor.reset();
    while(C=(ConnectType*) CNodes[x].right_successor.getnext()){
      Cc=new int;
      *Cc=CNodes[C->ind].UID;
      gni->successors.insert(Cc);
    }
    gni->node=x;
    if(CNodes[x].models.count()>0)
      gni->type=MNODE_TYPE;
    else
      gni->type=3;
    
    level->insert(gni);
  }

  for(x=0;x<ht.count();x++){
    level=(List*) ht.get(x);
    data->insert(level);
  }

  ht.clear2();
  return data;
}


int*
NetWork::distributionPartials(int& count){
 
  count=StatLevelCount;
  return Token::StatLevel;

}



void
NetWork::statistic(char* name){
  int x,y,l,z,depth;
  FILE* file;
  char out[1024];
  char st[256];
  int *T,m;
  int sumTry,sumPassed,averageTry,averagePassed,sumSteps;

#ifndef STUDIES

  cout << "Program was not compiled with the STUDIES variable set, try again\n";

#else

  if(NNodeX==0) return;
  if(CNodeX==0) return;
  if(MNodeX==0) return;

  file=fopen(name,"w");

  strcpy(out,"\n\n\n\n**************NETWORK PERFORMANCE***************\n\n");
  fputs(out,file);

  depth=0;
  l=0;
  
  for(x=0;x<MNodeX;x++){
    Token* tok;
    tok=(Token*) InfoMN[x].originalEdges.get(0);
    if(tok->length()>l) l=tok->length();

    if(MNodes[x].TopNumber>depth) depth=MNodes[x].TopNumber;
  }

  strcpy(out,"Max Model: ");
  sprintf(st,"%d%",l);
  strcat(out,st);
  strcat(out,"  Depth: ");
  sprintf(st,"%d",depth);
  strcat(out,st);
  strcat(out,"\n");

  fputs(out,file);

  sumTry=0;
  sumPassed=0;
  for(x=0;x<NNodeX;x++){
    sumTry+=NNodes[x].testTry;
    sumPassed+=NNodes[x].testPassed;
  }

  averageTry=sumTry/NNodeX;
  averagePassed=sumPassed/NNodeX;

  sprintf(st,"%d",NNodeX);
  strcpy(out,st);
  strcat(out,"  NNodes Tests  T:");
  sprintf(st,"%d",sumTry);
  strcat(out,st);
  strcat(out,"    P:");
  sprintf(st,"%d",sumPassed);
  strcat(out,st);
  strcat(out,"     Average: ");
  sprintf(st,"%d",averageTry);
  strcat(out,st);
  strcat(out,"     ");
  sprintf(st,"%d",averagePassed);
  strcat(out,st);
  strcat(out,"\n");

  fputs(out,file);


  
  sumTry=0;
  sumPassed=0;
  sumSteps=0;
  for(x=0;x<CNodeX;x++){
    sumTry+=CNodes[x].testTry;
    sumPassed+=CNodes[x].testPassed;
    sumSteps+=CNodes[x].testSteps;
  }
  
  if(CNodeX>0){
    averageTry=sumTry/CNodeX;
    averagePassed=sumPassed/CNodeX;
  }

  sprintf(st,"%d",CNodeX);
  strcpy(out,st);
  strcat(out,"  CNodes Tests  T:");
  sprintf(st,"%d",sumTry);
  strcat(out,st);
  strcat(out,"    P:");
  sprintf(st,"%d",sumPassed);
  strcat(out,st);
  strcat(out," Steps:");
  sprintf(st,"%d",sumSteps);
  strcat(out,st);
  strcat(out,"     Average:");
  sprintf(st,"%d",averageTry);
  strcat(out,st);			 
  strcat(out,"     ");
  sprintf(st,"%d",averagePassed);
  strcat(out,st);
  strcat(out,"\n");

  fputs(out,file);


  sumTry=0;
  sumPassed=0;
  for(x=0;x<CNodeX;x++){
    Token *tok;
    int t;
//    tok=(Token*) InfoCN[x].originalEdges.get(0);
    tok=(Token*) InfoCN[x].originalNodes.get(0);
    if(tok){
	t=tok->length();
	sumPassed+=t;
	sumTry+=t*InfoCN[x].originalNodes.count();
    }
  }

  strcpy(out,"\nShareability of the NetWork: \n");
  fputs(out,file);
  strcpy(out,"Total of Edges:");
  sprintf(st,"%d",sumTry);
  strcat(out,st);
  strcat(out,"   Represented:");
  sprintf(st,"%d",sumPassed);
  strcat(out,st);
  strcat(out,"  ---> ratio: ");
  sprintf(st,"%f",double(sumPassed)/double(sumTry));
  strcat(out,st);

  fputs(out,file);

  strcpy(out,"\nGenerated Token: ");
  sprintf(st,"%d",Token::inst_count);
  strcat(out,st);
  strcat(out,"  memory (~int): ");
  sprintf(st,"%d",Token::sumOfLength);
  strcat(out,st);

  fputs(out,file);

  strcpy(out,"\nMaximum Error generated:");
  sprintf(st,"%f",Token::maxError);
  strcat(out,st);
  strcat(out,"\n\nBest Instances:\n");

  fputs(out,file);

  if(CNodeX>0){
    CurrentBestTry.initNext();
    
    while(T=(int*) CurrentBestTry.getnext(&m)){
      Token *tok;
      
      if(CNodes[*T].GO.count()==0) continue;
      tok=(Token*) CNodes[*T].GO.get(0);
      
      strcpy(out,"Model:");
      sprintf(st,"%d",m);
      strcat(out,st);
      strcat(out,"  depth:");
      sprintf(st,"%d",CNodes[*T].TopNumber);
      strcat(out,st);
      strcat(out,"  Error :");
      sprintf(st,"%f",tok->totalErr());
      strcat(out,st);
      strcat(out," Future: ");
      sprintf(st,"%f",CNodes[*T].assignedFUTerr);
      strcat(out,st);
      strcat(out," Total: ");
      sprintf(st,"%f",tok->totalErr()+CNodes[*T].assignedFUTerr);
      strcat(out,st);
      strcat(out,"\n");
      fputs(out,file);
    }
  }
  strcpy(out,"\nTicks:");
  sprintf(st,"%d",time());
  strcat(out,st);
  fputs(out,file);

  fputs("\n\n\n",file);


  fclose(file);


#endif

}




void
NetWork::consistencyCheck(){

  int x,y;
  ConnectType *C;

/** parent - sons relationshipp **/

  for(x=0;x<NNodeX;x++){
    
    NNodes[x].left_successor.reset();
    while(C=(ConnectType*) NNodes[x].left_successor.getnext()){
      if(C->direction==0){
	gmt_assert(CNodes[C->ind].parents[0].ind==x);
      }else{
	gmt_assert(CNodes[C->ind].parents[1].ind==x);
	gmt_assert(0);
      }
    }
    NNodes[x].right_successor.reset();
    while(C=(ConnectType*) NNodes[x].right_successor.getnext()){
      if(C->direction==0){
	gmt_assert(CNodes[C->ind].parents[0].ind==x);
	gmt_assert(0);
      }else{
	gmt_assert(CNodes[C->ind].parents[1].ind==x);
      }
    }

  }
      
  for(x=0;x<NNodeX;x++){
    
    NNodes[x].left_successor.reset();
    while(C=(ConnectType*)NNodes[x].left_successor.getnext()){
      if(C->direction==0){
	gmt_assert(CNodes[C->ind].parents[0].NType==1);
	gmt_assert(CNodes[C->ind].parents[0].ind==x);
      }else{
	gmt_assert(CNodes[C->ind].parents[1].NType==1);
	gmt_assert(CNodes[C->ind].parents[1].ind==x);
	gmt_assert(0);
      }
    }
    NNodes[x].right_successor.reset();
    while(C=(ConnectType*)NNodes[x].right_successor.getnext()){
      if(C->direction==0){
	gmt_assert(CNodes[C->ind].parents[0].NType==1);
	gmt_assert(CNodes[C->ind].parents[0].ind==x);
	gmt_assert(0);
      }else{
	gmt_assert(CNodes[C->ind].parents[1].NType==1);
	gmt_assert(CNodes[C->ind].parents[1].ind==x);
      }
    }


  } 

 for(x=0;x<CNodeX;x++){
    
    CNodes[x].left_successor.reset();
    while(C=(ConnectType*)CNodes[x].left_successor.getnext()){
      if(C->direction==0){
	gmt_assert(CNodes[C->ind].parents[0].NType==3);
	gmt_assert(CNodes[C->ind].parents[0].ind==x);
      }else{
	gmt_assert(CNodes[C->ind].parents[1].NType==3);
	gmt_assert(CNodes[C->ind].parents[1].ind==x);
	gmt_assert(0);
      }
    }
   CNodes[x].right_successor.reset();
    while(C=(ConnectType*)CNodes[x].right_successor.getnext()){
      if(C->direction==0){
	gmt_assert(CNodes[C->ind].parents[0].NType==3);
	gmt_assert(CNodes[C->ind].parents[0].ind==x);
	gmt_assert(0);
      }else{
	gmt_assert(CNodes[C->ind].parents[1].NType==3);
	gmt_assert(CNodes[C->ind].parents[1].ind==x);
      }
    }

  }
}




/*** Balanced Compilation Supporting Cast  ****/


int
NetWork::isUsedInBalanced(Token* tok,int auid){
  int x,y;

  if(!tok) return 0;

  y=tok->length();

  for(x=0;x<y;x++){
    if(VX[(*tok)[x]]==-1) continue;
    if(VX[(*tok)[x]]!=auid) return 0;
  }  

  return 1;
}


int 
NetWork::setPartBalancedList(ConnectType *C){
  int x,input,key,auid;
  Token* tok;

  input=0;
  if(C->NType==NNODE_TYPE){
    NNodes[C->ind].GO.reset();
    while(tok=(Token*) NNodes[C->ind].GO.getnext()){
      if(isUsedInBalanced(tok,NNodes[C->ind].UID)){
	for(x=0;x<tok->length();x++) 
	  VX[(*tok)[x]]=NNodes[C->ind].UID;
	input=1;
	key=NNodes[C->ind].TopNumber;
	auid=NNodes[C->ind].UID;
      }
    }
  }else{
    CNodes[C->ind].GO.reset();
    while(tok=(Token*) CNodes[C->ind].GO.getnext()){
      if(isUsedInBalanced(tok,CNodes[C->ind].UID)){
	for(x=0;x<tok->length();x++) 
	  VX[(*tok)[x]]=CNodes[C->ind].UID;
	input=1;
	key=CNodes[C->ind].TopNumber;
	auid=CNodes[C->ind].UID;
      }
    }
  }
  
  if(input) CompileBalanced.insert(C,auid,key);
  
  return input;
}


void
NetWork::setBalancedList(){
  ConnectType* C;
  int k,x;

  /** clear EX list **/

  for(x=0;x<WModel->numberOfVertices();x++)
    VX[x]=-1;


  /** set it from CompileBest **/
  

  
  /** find largest clique of non intersecting entries in list **/
  
  while(CompileBest.count()>0){
    C=(ConnectType*) CompileBest.removeTop(&k);
    if(C->NType==NNODE_TYPE){
      findEdgeClique(&NNodes[C->ind].GO,NNodes[C->ind].UID,NNodes[C->ind].TopNumber,C);
    }else{
      findEdgeClique(&CNodes[C->ind].GO,CNodes[C->ind].UID,CNodes[C->ind].TopNumber,C);
    }
    
  }
} 



void
NetWork::markTokenForBalanced(int type,int node){
  int x;
  Token* tok;

  if(type==NNODE_TYPE){
    NNodes[node].GO.reset();
    while(tok=(Token*) NNodes[node].GO.getnext()){
      for(x=0;x<tok->length();x++) 
	VX[(*tok)[x]]=NNodes[node].UID;
    }
  }else if(CNODE_TYPE){
    CNodes[node].GO.reset();
    while(tok=(Token*) CNodes[node].GO.getnext()){
      for(x=0;x<tok->length();x++) 
	VX[(*tok)[x]]=CNodes[node].UID;
    }
  }
}





int
NetWork::time(){
  return endTime-startTime;
}



void
NetWork::findEdgeClique(List* edgego,int uid,int key,ConnectType* C){
  int** mx;
  int x,y,d,good,i;
  int* Best,*Current;
  int Besti,Currenti;
  List Trial;

  Token** K;
  Token* tok,*htok;




  mx=new int*[edgego->count()];
  K=new Token*[edgego->count()];
  List tmp;

  for(x=0;x<edgego->count();x++){
    mx[x]=new int[edgego->count()];
    for(y=0;y<edgego->count();y++) mx[x][y]=1;
  }

  edgego->reset();
  i=0;
  while(edgego->count()){
    tok=(Token*) edgego->remove(0);
    if(isUsedInBalanced(tok,uid)){
      K[i]=tok;
      i++;
    }else{
      tmp.insert(tok);
    }
  }
  
  if(i>0){
    
    for(x=0;x<i;x++){
   
      for(y=0;y<i;y++){
	if(x==y) continue;
	
	mx[x][y]=!(K[x]->intersect(K[y]));
      }
    }
    
    Current=new int[i];
    Best=new int[i];
    
    for(x=0;x<i;x++){
      Current[x]=0;
      Best[x]=0;
      Besti=0;
      Currenti=0;
      
    }
    
    for(x=0;x<i;x++) Clique(x,Best,Current,Besti,Currenti,i,mx);
    
    (*edgego)=tmp;

    for(x=0;x<i;x++){
      if(Best[x]) {
	for(y=0;y<K[x]->length();y++) 
	  VX[(*K[x])[y]]=uid;
      
      edgego->insert(K[x]);
      }
    }
    


    delete Best;
    delete Current;
  

    CompileBalanced.insert(C,uid,key);
    
  }else{
    
    (*edgego)=tmp;
    delete C;
  }


  for(x=0;x<edgego->count();x++) delete mx[x];
  delete mx;
  delete K;


}



void
NetWork::displayTreeAscii(){
  int m,x,y,l,k,j,Level,flag,s;
  int *i;
  Token* org,*tok,*otok,*oetok;
  int* POS,*Order;
  SortedList ORD;

#if 0
  for(m=0;m<ModelDataNumber;m++){

    org=ModelData[m].AM->orderCoherent();

    if(org->length()>30) return;

    POS=new int[org->length()];
    Order=new int[org->length()];
    for(x=0;x<org->length();x++) POS[(*org)[x]]=x;

    cout << "Model " << m << "\n";
    for(x=0;x<org->length();x++) printf("%4d",(*org)[x]);
    printf("\n");

    for(x=0;x<NNodeX;x++){
      if(InfoNN[x].getOriginals(m,&otok,&oetok)){

	for(y=0;y<otok->length();y++){
	  i=new int;
	  *i=y;
	  ORD.insert(i,y,POS[(*otok)[y]]);
	}

	j=0;
	while(ORD.count()>0){
	  i=(int*) ORD.removeTop(&l);
	  Order[j]=l;
	  delete i;
	  j++;
	}

	NNodes[x].GO.reset();
	while(tok=(Token*) NNodes[x].GO.getnext()){

	  for(l=0;l<otok->length();l++){
	    j=Order[l];
	    if(l>0) s=POS[(*otok)[Order[l-1]]]; else s=0;
	    for(y=s;y<POS[(*otok)[j]];y++) printf("    ");
	
	    printf("%4d",(*tok)[j]);
	  }
	  printf("\n");
	}
	
      }
    }

 
     
    
    for(Level=2;Level<CNodeX;Level++){

      flag=0;
      
      for(x=0;x<CNodeX;x++){

	if(CNodes[x].TopNumber==Level){
	  flag=1;

	  if(InfoCN[x].getOriginals(m,&otok,&oetok)){

	    for(y=0;y<otok->length();y++){
	      i=new int;
	      *i=y;
	      ORD.insert(i,y,POS[(*otok)[y]]);
	    }
	    
	    j=0;
	    while(ORD.count()>0){
	      i=(int*) ORD.removeTop(&l);
	      Order[j]=l;
	      delete i;
	      j++;
	    }
	    
	    CNodes[x].GO.reset();
	    while(tok=(Token*) CNodes[x].GO.getnext()){
	      
	      for(l=0;l<otok->length();l++){
		j=Order[l];
		if(l>0) s=POS[(*otok)[Order[l-1]]]+1; else s=0;
		for(y=s;y<POS[(*otok)[j]];y++) printf("    ");
		
		printf("%4d",(*tok)[j]);
	      }
	      printf("\n");
	    }
	  }
	}
      }
      if(flag==0) break;
    }


  
    delete POS;
    delete Order;

  }
#endif
}


void
NetWork::prepareOPENandSTACK(){
  int x;
  action_element *P;
  ConnectType* CT;
  
  CT=(ConnectType*) MNodes[0].parents.get(0);
  
  recursiveSTACKfill(CT);

  prepareOPEN();
  
}

void
NetWork::recursiveSTACKfill(ConnectType* CT){
 
  
  if(CT->NType==CNODE_TYPE){

    if(CNodes[CT->ind].GO.count()==0){
      STACK.push(CNodes[CT->ind].UID,CT);
      
      recursiveSTACKfill(&CNodes[CT->ind].parents[0]);
      recursiveSTACKfill(&CNodes[CT->ind].parents[1]);
    }
  }
  
 
} 


void
NetWork::prepareOPEN(){
  int x,key;
  ConnectType* CT;
  Token* tok;

  CT=(ConnectType*) STACK.pop(&key);
  if(CT==NULL) return;

  OPEN.clear();
  ACTIVE.clear();
  
  CONTROL_GO=&CNodes[CT->ind].GO;
  recursiveOPENfill(CT);
    
  if(CT->NType==CNODE_TYPE)
    x=CNodes[CT->ind].UID;
  else
    x=NNodes[CT->ind].UID;

  cout << "\ndebug:  Active Node: " << x << "\n";  

}


void
NetWork::recursiveOPENfill(ConnectType* CT){
  Token* tok;
  int key;

  if(CT==NULL) return;
  if(CT->NType==NNODE_TYPE){
    ACTIVE.insert(NULL,NNodes[CT->ind].UID);
    tok=(Token*) NNodes[CT->ind].STOP.top(&key);
    insertIntoOPEN(CT->ind,CT->NType,NNodes[CT->ind].UID,tok);
    
  }
  if(CT->NType==CNODE_TYPE){
    recursiveOPENfill(&CNodes[CT->ind].parents[0]);
    recursiveOPENfill(&CNodes[CT->ind].parents[1]);
    ACTIVE.insert(NULL,CNodes[CT->ind].UID);
    tok=(Token*) CNodes[CT->ind].STOP.top(&key);
    insertIntoOPEN(CT->ind,CT->NType,CNodes[CT->ind].UID,tok);
    
  }  
}

/***************************************************/
/* flexible compilation from MetaGraph             */
/***************************************************/


void
NetWork::buildFromMetaGraph(List *Ml,int modelStartNumber){
  MetaGraph *NC;
  In_MetaGraph* G;
  int v,e;

  v=0;
  e=0;
  Ml->reset();
  while(G=(In_MetaGraph*) Ml->getnext()){
    WModel=&G->graph->AM;
    v+=WModel->numberOfVertices();
    e+=WModel->numberOfEdges();
  }

  reallocNodes(v,e,Ml->count());

  Ml->reset();
  WMNumber=modelStartNumber;
  while(G=(In_MetaGraph*) Ml->getnext()){
    WModel=&G->graph->AM;
    
   
    NC=buildNetwork(G);
    
    if(NC!=NULL){
      //create model node
      
      Token *orig,*eorig;
      int *CI;  
      ConnectType* CP;
      
      MNodes[MNodeX].modelNr=WMNumber;
      MNodes[MNodeX].UID=UIDLimit;
      UIDLimit++;
      CP=new ConnectType;
      CP->NType=NC->type;
      CP->ind=NC->node;
      MNodes[MNodeX].parents.insert(CP);  
      orig=(Token*) G->tok;
      MNodes[MNodeX].Original=*(orig);
      MNodes[MNodeX].TopNumber=CNodes[NC->node].TopNumber;
      
      
      eorig=new  Token;
      (*eorig)=(*orig);
      InfoMN[MNodeX].originalEdges.insert(eorig);
      
      CI=new int;
      *CI=MNodeX;
      CNodes[NC->node].models.insert(CI);
      MNodeX++;
      WMNumber++;
    }
  }
  WModel=NULL;
}



MetaGraph*
NetWork::buildNetwork(In_MetaGraph* G){
// recursive function
  MetaGraph* M1,*M2;
  In_MetaGraph* G1,*G2;
  int x;
  ConnectType* C;

  
  gmt_assert(G->parts.count()<3);
  


  if(G->parts.count()==1){
    G->MyGraph=buildNetwork((In_MetaGraph*) G->parts.get(0));
    return G->MyGraph;
  }
  
  if(G->parts.count()==2){
    // create CNodes
    
    
    G1=(In_MetaGraph*) G->parts.get(0);   // LEFT  PARENT
    G2=(In_MetaGraph*) G->parts.get(1);   // RIGHT PARENT
  
    if(G->MyGraph->represented==1){
      G1->MyGraph->represented=1;
      G2->MyGraph->represented=1;
      ConnectType C=G->MyGraph->Node->parents[0];
      G1->MyGraph->node=C.ind;
      G1->MyGraph->type=C.NType;
      if(C.NType==NNODE_TYPE) G1->MyGraph->Node=&NNodes[C.ind];
      else G1->MyGraph->Node=&CNodes[C.ind];
      
      C=G->MyGraph->Node->parents[1];
      G2->MyGraph->node=C.ind;
      G2->MyGraph->type=C.NType;
      if(C.NType==NNODE_TYPE) G2->MyGraph->Node=&NNodes[C.ind];
      else G2->MyGraph->Node=&CNodes[C.ind];
    
    }

    buildNetwork(G1);
    buildNetwork(G2);
    
    if(G->MyGraph->represented==0){
      C=createSubgraphMatcher(G,G1,G2);
      
      G->MyGraph->Node=&CNodes[CNodeX-1];
    }else{
      Token *tok;
      tok=new Token;
      int i=G->MyGraph->node;
      (*tok)=(*G->tok);
      gmt_assert(G->MyGraph->type==CNODE_TYPE);
      gmt_assert(tok->length()==CNodes[i].newLength);
      InfoCN[i].originalNodes.insert(tok);
      
      int *m;
      m=new int;
      *m=WMNumber;
      InfoCN[i].originalModels.insert(m);
      
    }
  }
  
  if(G->MyGraph->represented==0){
    if(G->parts.count()==0){
      // create NNodes
      
      C=createVertexChecker(G);
      G->MyGraph->Node=&NNodes[NNodeX-1];
    }
      
    G->MyGraph->represented=1;
    G->MyGraph->node=C->ind;
    G->MyGraph->type=C->NType;
      
  }
  

  return G->MyGraph;
  
}


ConnectType* 
NetWork::createSubgraphMatcher(In_MetaGraph* G,In_MetaGraph *G1,In_MetaGraph* G2){
  ConnectType* C;
  NNodeType* N1,*N2;
  int i;
  List Elist;
  
  if(G1->MyGraph->type==NNODE_TYPE)
    N1=&NNodes[G1->MyGraph->node];
  else
    N1=&CNodes[G1->MyGraph->node];

  if(G2->MyGraph->type==NNODE_TYPE)
    N2=&NNodes[G2->MyGraph->node];
  else
    N2=&CNodes[G2->MyGraph->node];

  C=new ConnectType;
  i=CNodeX;

  WModel=&G->graph->AM;

  findBridges(G1->tok,G2->tok,&Elist);

  CNodes[i].testList=Elist;

  CNodes[i].newLength=N1->newLength+N2->newLength;

  gmt_assert(CNodes[i].newLength==G->tok->length());

  CNodes[i].parents[0].ind=G1->MyGraph->node;
  CNodes[i].parents[0].NType=G1->MyGraph->type;
  CNodes[i].parents[0].direction=LEFT;
  C=new ConnectType;
  C->ind=i;
  C->NType=3;
  C->direction=LEFT;
  N1->left_successor.insert(C);
  N1->represented++;

  CNodes[i].parents[1].ind=G2->MyGraph->node;
  CNodes[i].parents[1].NType=G2->MyGraph->type;
  CNodes[i].parents[1].direction=RIGHT;
  C=new ConnectType;
  C->ind=i;
  C->NType=3;
  C->direction=RIGHT;
  N2->right_successor.insert(C);
  N2->represented++;

  CNodes[i].UID=UIDLimit;
  UIDLimit++;
  CNodes[i].totalEdges=Elist.count();


  if(N1->TopNumber>N2->TopNumber)
    CNodes[i].TopNumber=N1->TopNumber+1;
  else
    CNodes[i].TopNumber=N2->TopNumber+1;

  CNodes[i].totalEdges+=N1->totalEdges+N2->totalEdges;

  Token *tok;
  tok=new Token;
  (*tok)=(*G->tok);
  InfoCN[i].originalNodes.insert(tok);

  int *m;
  m=new int;
  *m=WMNumber;
  InfoCN[i].originalModels.insert(m);


  CNodeX++;

  C=new ConnectType;
  C->ind=i;
  C->NType=3;
  return C;
}


ConnectType*
NetWork::createVertexChecker(In_MetaGraph* G){
  ConnectType *C;
  AdjazenzMatrix* graph;
  AttId at;
  int edg;

  graph=&G->graph->AM;

  C=new ConnectType;

  NNodes[NNodeX].TestID = graph->getNodeAttributeId(G->vertex);

  NNodes[NNodeX].totalDeletionCost=ATTC->deletionCost(NNodes[NNodeX].TestID); 
  graph->initNext(G->vertex,G->vertex);
  while((at=graph->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
    BridgeData* bd;
    bd=new BridgeData;
    bd->n1=G->vertex;
    bd->n2=G->vertex;
    bd->id=at;
    NNodes[NNodeX].totalDeletionCost+=ATTC->deletionCost(bd->id);
    NNodes[NNodeX].testList.insert(bd);
  }
  
  
  NNodes[NNodeX].TopNumber=0;
  NNodes[NNodeX].UID=UIDLimit;

  Token *tok;
  tok=new Token;
  
  (*tok)=(*G->tok);
  InfoNN[NNodeX].originalNodes.insert(tok);
  int *m;
  m=new int;
  *m=WMNumber;
  InfoNN[NNodeX].originalModels.insert(m);


  UIDLimit++;
  
  C->NType=1;
  C->ind=NNodeX;
  NNodeX++;
  return C;
}


/*****end flexible compilation from MetaGraph******/



int
NetWork::averageFUTerr(NNodeType* Node,Token* tok){
  int x;
  double l_err=0;
  double r_err=0;  

  if(Node->top_instances.count()<Node->represented){
    if(Node->TopNumber>0){
      if(Node->parents[0].NType==NNODE_TYPE) l_err=NNodes[Node->parents[0].ind].FUTerr_average;
      else l_err=CNodes[Node->parents[0].ind].FUTerr_average;
      if(Node->parents[1].NType==NNODE_TYPE) r_err=NNodes[Node->parents[0].ind].FUTerr_average;
      else r_err=CNodes[Node->parents[1].ind].FUTerr_average;
      l_err+=r_err;
      l_err=l_err/Node->represented;
    }

    Node->FUTerr_average=0;
    double *err;
    if(tok){
      err=new double;
      *err=tok->totalErr();
      Node->top_instances.insert(err);
    }      
    for(x=0;x<Node->top_instances.count();x++){
      err=(double*) Node->top_instances.get(x);
      Node->FUTerr_average+=(*err);
    }
    if(Node->top_instances.count()<Node->represented){
      Node->FUTerr_average+=(Node->represented-Node->top_instances.count())*Node->FUT;
    }
    Node->FUTerr_average=Node->FUTerr_average/Node->represented;
    
    if(l_err>Node->FUTerr_average) Node->FUTerr_average=l_err; 
    return 1;
  }
  return 0;
}





double
NetWork::getSecondHeuristic(ConnectType& C_from,ConnectType& C_to){
  double err,minus;
  if(C_from.NType==NNODE_TYPE)
    err=NNodes[C_from.ind].FUTerr_average;
  if(C_from.NType==CNODE_TYPE)
    err=CNodes[C_from.ind].FUTerr_average;
  if(C_to.NType==NNODE_TYPE)
    minus=(NNodes[C_to.ind].represented-1)*NNodes[C_to.ind].FUTerr_average;
  if(C_to.NType==CNODE_TYPE)
    minus=(CNodes[C_to.ind].represented-1)*CNodes[C_to.ind].FUTerr_average;
 
  err-=minus;
  return err;
}






double
NetWork::individualLookahead(Token* tok, int node, int type){
  Token *otok,*mtok;
  int* AVAIL;
  int x,y,p0,p1;
  int *m;
  double err,err2;
  err=INFTY_COST;
  AVAIL=new int[WModel->numberOfVertices()];
  for(x=0;x<WModel->numberOfVertices();x++)
    AVAIL[x]=1;
  
  for(x=0;x<tok->length();x++)
    if((*tok)[x]>-1)
      AVAIL[(*tok)[x]]=0;
  
  p0=-1;
  for(x=0;x<InfoCN[node].originalNodes.count();x++){
    otok=(Token*) InfoCN[node].originalNodes.get(x);
    m=(int*) InfoCN[node].originalModels.get(x);
    
    mtok=&MNodes[*m].Original;
    for(y=0;y<mtok->length();y++){
      if((*mtok)[y]==(*otok)[0]){
	p0=y;
	p1=p0+otok->length();
	break;
      }
    }
    gmt_assert(p0>-1);
    err2=Lookahead_result(mtok,p0,p1,tok,&ModelData[*m].G->AM,WModel,AVAIL,ATTC);
    if(err>err2) err=err2;
    if(err==0) break;
  }
  tok->tot_err+=err;
  delete[] AVAIL;
  return err;
}



int
NetWork::renewEstimation(action_element* P){
  double err,derr;
  
  err=P->tok->totalErr();
  P->tok->calcError();
  derr=err-P->tok->totalErr();
  
  if(derr<CNodes[P->node].assignedFUTerr){
    P->tok->tot_err+=CNodes[P->node].assignedFUTerr;
    return 0;
  }
   
  return 1;
  
}
