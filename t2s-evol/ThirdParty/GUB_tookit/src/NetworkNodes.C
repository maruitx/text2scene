#include "ComNet.h"
#include "gmt_assert.h"


/*************************************/
/* Note the differences between      */
/* ~NNodes(),clear(),discard()       */
/*************************************/

extern Token* DUMMY_TOKEN;

NNodeType::NNodeType(){
  
#ifdef STUDIES
  testTry=0;
  testPassed=0;
#endif

  newLength=1;
  terminal=0;
  FutErrorToken=NULL;
  totalEdges=0;
  totalDeletionCost=0;
  FUTerr=0;
  FUT=0;
  assignedFUTerr=MAX_COST;
  represented=0;
  FUTerr_average=0;
}


NNodeType::~NNodeType(){
  shallowClear();
  left_successor.clear();
  right_successor.clear();
  models.clear();
  testList.clear();
  
}



NNodeType::NNodeType(NNodeType &n){
  
  GO=n.GO;
  TMP_GO=n.TMP_GO;
  STOP=n.STOP;
  STOP2=n.STOP;

  TestID=n.TestID;
  TopNumber=n.TopNumber;
  UID=n.UID;
  left_successor=n.left_successor;
  right_successor=n.right_successor;
  parents[0]=n.parents[0];
  parents[1]=n.parents[1];
  models=n.models;

 
  assignedFUTerr=n.assignedFUTerr;
  FUTerr=n.FUTerr;
  FUT=n.FUT;
  FUTlist=n.FUTlist;

  FutErrorToken=n.FutErrorToken;
  totalEdges=n.totalEdges;
  totalDeletionCost=n.totalDeletionCost;
 
  testList=n.testList;
  represented=n.represented;
  top_instances=n.top_instances;

#ifdef STUDIES
  testTry=0;
  testPassed=0;
#endif

}
 
 
NNodeType
NNodeType::operator=(NNodeType &n){
 

  GO=n.GO;
  TMP_GO=n.TMP_GO;
  STOP=n.STOP;
  STOP2=n.STOP2;
  TestID=n.TestID;
  TopNumber=n.TopNumber;
  UID=n.UID;
  left_successor=n.left_successor;
  right_successor=n.right_successor;
  parents[0]=n.parents[0];
  parents[1]=n.parents[1];
  models=n.models;

  assignedFUTerr=n.assignedFUTerr;
  FUTerr=n.FUTerr;
  FUT=n.FUT;
  FUTlist=n.FUTlist;
  

  FutErrorToken=n.FutErrorToken;
  totalDeletionCost=n.totalDeletionCost;
  totalEdges=n.totalEdges;
 
  represented=n.represented;
  top_instances=n.top_instances;

#ifdef STUDIES
  testTry=0;
  testPassed=0;
#endif

  terminal=0;
  testList=n.testList;

  return *this;
  
}

void
NNodeType::shallowClear(){
  terminal=0;
  GO.clear();
  STOP.clear();
  STOP2.clear();
  FUTlist.clear();
  TMP_GO.clear();
}

void
NNodeType::clear(){
  Token* tok;
  int k;
  ConnectType* Q;

#ifdef STUDIES
  testTry=0;
  testPassed=0;
#endif  

  terminal=0;
  
  while(GO.count()){
    tok=(Token*) GO.remove(0);
    delete tok;
  }

  STOP2.clear();

  while(STOP.count()){
    tok=(Token*) STOP.removeTop(&k);
    delete tok;
  }


  GO.statMax=0;
  GO.statTotal=0;

  while(FUTlist.count()>0){
    Q=(ConnectType*) FUTlist.removeTop(&k);
    delete Q;
  }

  EdgeGO.statMax=0;
  EdgeGO.statTotal=0;

  if(FutErrorToken) 
    if(FutErrorToken!=DUMMY_TOKEN)
      delete FutErrorToken;
  FutErrorToken=NULL;

  FUTerr=0;
  FUT=0;
  assignedFUTerr=MAX_COST;

  STOP.statMax=0;
  STOP.statTotal=0;

  TMP_GO.clear();

  top_instances.clear();
  
}

void
NNodeType::discard(){
  ConnectType* C;
  int *m;


  clear();

  while(left_successor.count()){
    C=(ConnectType*) left_successor.remove(0);
    delete C;
  }

  while(right_successor.count()){
    C=(ConnectType*) right_successor.remove(0);
    delete C;
  }
  
  while(models.count()){
    m=(int*) models.remove(0);
    delete m;
  }


  while(testList.count()>0){
    BridgeData* bd=(BridgeData*) testList.remove(0);
    delete bd;
  }

  testList.clear();

}



void 
NNodeType::write(FILE *file){
  int n,*i,x;
  ConnectType* C;
  
#ifdef NEW_ATTOBJECT
  TestID.write(file);
#else
  fwrite((char*)&TestID,sizeof(AttId),1,file);
#endif 
  fwrite((char*)&TopNumber,sizeof(int),1,file);
  fwrite((char*)&UID,sizeof(int),1,file);
  fwrite((char*)&totalEdges,sizeof(int),1,file);
  fwrite((char*)&totalDeletionCost,sizeof(double),1,file);
  fwrite((char*)parents,sizeof(ConnectType),2,file);
  
  n=left_successor.count();
  fwrite((char*)&n,sizeof(int),1,file);
  
  for(x=0;x<n;x++){
    C=(ConnectType*) left_successor.get(x);
    fwrite((char*)C,sizeof(ConnectType),1,file);
  }
  
  n=right_successor.count();
  fwrite((char*)&n,sizeof(int),1,file);
  
  for(x=0;x<n;x++){
    C=(ConnectType*) right_successor.get(x);
    fwrite((char*)C,sizeof(ConnectType),1,file);
  }
  
  n=models.count();
  fwrite((char*)&n,sizeof(int),1,file);
  
  for(x=0;x<n;x++){
    i=(int*) models.get(x);
    fwrite((char*)i,sizeof(int),1,file);
  }
  

  int l=testList.count();
  fwrite((char*)&l,sizeof(int),1,file);

  for(x=0;x<l;x++){
    BridgeData* bd=(BridgeData*)testList.get(x);
    fwrite((char*)bd,sizeof(BridgeData),1,file);
#ifdef NEW_ATTOBJECT
    bd->id.write(file);
#endif
  }
  
  n=represented;
  fwrite((char*)&n,sizeof(int),1,file);
  
  
}


void 
NNodeType::read(FILE *file){
  int x,n,*i,l;
  ConnectType* C;
  
  discard();
#ifdef NEW_ATTOBJECT
  TestID.read(file);
#else
  fread((char*)&TestID,sizeof(AttId),1,file);
#endif 
  fread((char*)&TopNumber,sizeof(int),1,file);
  fread((char*)&UID,sizeof(int),1,file);
  fread((char*)&totalEdges,sizeof(int),1,file);
  fread((char*)&totalDeletionCost,sizeof(double),1,file);
  fread((char*)parents,sizeof(ConnectType),2,file);

  fread((char*)&n,sizeof(int),1,file);

  for(x=0;x<n;x++){
    C=new ConnectType;
    fread((char*)C,sizeof(ConnectType),1,file);
    left_successor.insert(C);    
  }
  
  fread((char*)&n,sizeof(int),1,file);
  
  for(x=0;x<n;x++){ 
    C=new ConnectType;
    fread((char*)C,sizeof(ConnectType),1,file);
    right_successor.insert(C);  
  }

  
  fread((char*)&n,sizeof(int),1,file);
  
  for(x=0;x<n;x++){
    i=new int;
    fread((char*)i,sizeof(int),1,file);
    models.insert(i);
  }

  testList.clear();
  fread((char*)&l,sizeof(int),1,file);
  
  for(x=0;x<l;x++){
    BridgeData *bd;
    bd=new BridgeData;
    fread((char*)bd,sizeof(BridgeData),1,file);
#ifdef NEW_ATTOBJECT
    bd->id.read(file);
#endif
    testList.insert(bd);
  }
  
  fread((char*)&represented,sizeof(int),1,file);
}



void
NNodeType::dump(){
  ConnectType *C; 
  int x;
  
  cout << "UID: " << UID << "\n";
  
  cout << "GO>";
  
  std::string a = "Token";
  char* ca = new char[a.size() + 1];
  std::copy(a.begin(), a.end(), ca);
  ca[a.size()] = '\0';

  GO.dump(ca);
  
  cout << "Edges>";
  
  EdgeGO.dump(ca);
  
  cout << "STOP>";
  
  STOP.dump(ca);
  
  
  
  cout << "Parents>";
  cout << parents[0].ind << " :: " << parents[1].ind;
  
  cout << "Left Succ: " << left_successor.count();
  cout <<" >( ";
  for(x=0;x<left_successor.count();x++){
    C=(ConnectType*) left_successor.get(x);
    cout << C->ind << "  ";
  }
  cout << ")\n";
  
  cout << "Right Succ: " << right_successor.count();
  cout <<" >( ";
  for(x=0;x<right_successor.count();x++){
    C=(ConnectType*) right_successor.get(x);
    cout << C->ind << "  ";
  }
  cout << ")\n";
  
}



void
NNodeType::dump(FILE *file){
  ConnectType *C; 
  int x,y,k;
  char out[4048];
  char st[256];
  Token* tok;
 
  
  strcpy(out,"\nUID:");
  sprintf(st,"%d",UID);
  strcat(out,st);
  strcat(out,"\n");

  fputs(out,file);

  strcpy(out,"GO[");
  sprintf(st,"%d",GO.count());
  strcat(out,st);
  strcat(out,"]: ");
  
  GO.reset();
  for(x=0;x<GO.count();x++){
    tok=(Token*) GO.getnext();
    strcat(out,"(");
    for(y=0;y<tok->length();y++){
      if(y>0) strcat(out,",");
      sprintf(st,"%d",(*tok)[y]);
      strcat(out,st);
    }
    strcat(out,") ");
  }
  strcat(out,"\n");
  
  fputs(out,file);

  strcpy(out,"STOP[");
  sprintf(st,"%d",STOP.count());
  strcat(out,st);
  strcat(out,"]: ");	  
  STOP.initNext();
  for(x=0;x<STOP.count();x++){
    tok=(Token*) STOP.getnext(&k);
    strcat(out,"(");
    for(y=0;y<tok->length();y++){
      if(y>0) strcat(out,",");
      sprintf(st,"%d",(*tok)[y]);
      strcat(out,st);
    }
    strcat(out,") ");
  }
  strcat(out,"\n");

  fputs(out,file);
  
  strcpy(out,"LEFT[");
  sprintf(st,"%d",left_successor.count());
  strcat(out,st);
  strcat(out,"]: ");
  if(left_successor.count()>0) strcat(out,"(");
  for(x=0;x<left_successor.count();x++){
    if(x>0) strcat(out,",");
    C=(ConnectType*) left_successor.get(x);
    sprintf(st,"%d",C->ind);
    strcat(out,st);
  }

  if(left_successor.count()>0) strcat(out,")");
  strcat(out,"\n");
  fputs(out,file);

  strcpy(out,"RIGHT[");
  sprintf(st,"%d",right_successor.count());
  strcat(out,st);
  strcat(out,"]: ");

  if(right_successor.count()>0) strcat(out,"(");

  for(x=0;x<right_successor.count();x++){
    if(x>0) strcat(out,",");
    C=(ConnectType*) right_successor.get(x);
    sprintf(st,"%d",C->ind);
    strcat(out,st);
  }
  if(right_successor.count()>0) strcat(out,")");
  strcat(out,"\n");
  fputs(out,file);


    strcpy(out,"MODELS[");
  sprintf(st,"%d",models.count());
  strcat(out,st);
  strcat(out,"]: ");

  if(models.count()>0) strcat(out,"(");

  for(x=0;x<models.count();x++){
    int *I;
    if(x>0) strcat(out,",");
    I=(int*) models.get(x);
    sprintf(st,"%d",*I);
    strcat(out,st);
  }
  if(models.count()>0) strcat(out,")");
  strcat(out,"\n");
  fputs(out,file);

  sprintf(st,"%f",FUTerr);
  fputs("EstimatedError:",file);
  fputs(st,file);
 

  fputs("  Flag:",file);
  sprintf(st,"%d",FUT);
  fputs(st,file);
  fputs("\n",file);
  fputs("Assignd Error:",file);
  sprintf(st,"%f",assignedFUTerr);

  fputs(st,file);
  fputs("\n",file);


  strcpy(out,"     ~~~~~~~~~~~~~~~~~~~~~~~~~~~    \n");
  fputs(out,file);
  
}


/**** Cnodes      ****/


CNodeType::CNodeType()
:NNodeType(){

}


CNodeType::~CNodeType()
{
 

}


void
CNodeType::discard(){

  NNodeType::discard();
  
  
}



CNodeType::CNodeType(CNodeType &n)
:NNodeType(n)
{

  newLength=n.newLength;
  EdgeGO=n.EdgeGO;
 

  SGI=n.SGI;
 
}
 
 
CNodeType
CNodeType::operator=(CNodeType &n){
  
  NNodeType::operator=(n);
  newLength=n.newLength;
  EdgeGO=n.EdgeGO;

  SGI=n.SGI;

  return *this;
  
}


void
CNodeType::write(FILE *file){
  int l,x;
  Token *tok;

  NNodeType::write(file);

  fwrite((char*)&newLength,sizeof(int),1,file);

  l=SGI.count();
  fwrite((char*)&l,sizeof(int),1,file);

  SGI.reset();
  for(x=0;x<SGI.count();x++){
    tok=(Token*) SGI.getnext();
    tok->write(file);
  }

}


void
CNodeType::read(FILE* file){
  int l,x;
  Token* tok;

  NNodeType::read(file);

  fread((char*)&newLength,sizeof(int),1,file);
  
  fread((char*)&l,sizeof(int),1,file);

  for(x=0;x<l;x++){
    tok=new Token;
    tok->read(file);
    SGI.insert(tok);
  }

}





/** MNodeType     */

MNodeType::MNodeType(){

  modelName=new char[32];
  strcpy(modelName,"default");
}

MNodeType::~MNodeType(){
  
  if(modelName) delete[] modelName;
  clear();
  parents.clear();
 
}



MNodeType::MNodeType(MNodeType &n){
  parents=n.parents;
  modelNr=n.modelNr;
  TopNumber=n.TopNumber;
  UID=n.UID;

  if(n.modelName){
    modelName=new char[strlen(n.modelName)+1];
    strcpy(modelName,n.modelName);
  }
  Original=n.Original;
}
  


MNodeType 
MNodeType::operator=(MNodeType &n){
  parents=n.parents;
  modelNr=n.modelNr;
  TopNumber=n.TopNumber;
  UID=n.UID;

  if(modelName) delete[] modelName;
  modelName=NULL;

  if(n.modelName){
    modelName=new char[strlen(n.modelName)+1];
    strcpy(modelName,n.modelName);
  }
  Original=n.Original;
  return *this;
}


void
MNodeType::write(FILE *file){
 
  
  fwrite((char*)&modelNr,sizeof(int),1,file);
  fwrite((char*)&TopNumber,sizeof(int),1,file);
  fwrite((char*)&UID,sizeof(int),1,file);

#if 1
  ConnectType *C;
  C=(ConnectType*) parents.get(0);
  fwrite((char*)C,sizeof(ConnectType),1,file);
#endif  

  Original.write(file);

}


void
MNodeType::read(FILE* file){


  fread((char*)&modelNr,sizeof(int),1,file);
  fread((char*)&TopNumber,sizeof(int),1,file);
  fread((char*)&UID,sizeof(int),1,file);
  
  
#if 1
  ConnectType *C;
  C=new ConnectType;
  fread((char*)C,sizeof(ConnectType),1,file);
  parents.insert(C);
#endif
  
  Original.read(file);
}


void
MNodeType::clear(){
  int k;
  Token* tok;

  while(STOP.count()){
    tok=(Token*) STOP.removeTop(&k);
    delete tok;
  }

  while(instances.count()) {
    tok=(Token*) instances.remove(0);
    delete tok;
  }
}


void
MNodeType::discard(){
  ConnectType* C;

  clear();
  while(parents.count()>0){
    C=(ConnectType*) parents.remove(0);
    delete C;
  }

 if(modelName) delete modelName;
  modelName=NULL;
}

/**   InfoNodes  **/


NetInfoType::NetInfoType(){

}

NetInfoType::~NetInfoType(){
  originalNodes.clear();
  originalEdges.clear();
  originalModels.clear();
}


NetInfoType::NetInfoType(NetInfoType &n){
  originalNodes=n.originalNodes;
  originalEdges=n.originalEdges;
  originalModels=n.originalModels;
}


void
NetInfoType::discard(){
  Token* tok;
  int* i;

  while(originalNodes.count()>0){
    tok=(Token*) originalNodes.remove(0);
    delete tok;
  }
  while(originalEdges.count()>0){
    tok=(Token*) originalEdges.remove(0);
    delete tok;
  }
  
  while(originalModels.count()>0){
    i=(int*) originalModels.remove(0);
    delete i;
  }
}



NetInfoType
NetInfoType::operator=(NetInfoType &n){
  originalNodes=n.originalNodes;
  originalEdges=n.originalEdges;
  originalModels=n.originalModels;
  return *this;
}

void
NetInfoType::dump(FILE* file){
char st[256];
char out[1024];
int x,y;
Token* tok;

  strcpy(out,"\nORIGINALS[");
  sprintf(st,"%d",originalNodes.count());
  strcat(out,st);
  strcat(out,"]: ");

  originalNodes.reset();
  for(x=0;x<originalNodes.count();x++){
    tok=(Token*) originalNodes.getnext();
    strcat(out,"(");
    for(y=0;y<tok->length();y++){
      if(y>0) strcat(out,",");
      sprintf(st,"%d",(*tok)[y]);
      strcat(out,st);
    }
    strcat(out,") ");
  }
  if(originalNodes.count()>0) strcat(out,")");
  strcat(out,"\n");
  fputs(out,file);


}


void
NetInfoType::write(FILE* file){
  int x,l,*m;
  Token* tok;

  l=originalNodes.count();
  fwrite((char*)&l,sizeof(int),1,file);

  originalNodes.reset();
  for(x=0;x<originalNodes.count();x++){
    tok=(Token*) originalNodes.getnext();
    tok->write(file);
  }
    
  l=originalEdges.count();
  fwrite((char*)&l,sizeof(int),1,file);

  originalEdges.reset();
  for(x=0;x<originalEdges.count();x++){
    tok=(Token*) originalEdges.getnext();
    tok->write(file);
  }
  
  l=originalModels.count();
  fwrite((char*)&l,sizeof(int),1,file);

  originalModels.reset();
  for(x=0;x<originalModels.count();x++){
    m=(int*) originalModels.getnext();
    fwrite((char*)m,sizeof(int),1,file);
  }


}

void 
NetInfoType::read(FILE* file){
  int l,x,*m;
  Token* tok;

  fread((char*)&l,sizeof(int),1,file);

  for(x=0;x<l;x++){
    tok=new Token;
    tok->read(file);
    originalNodes.insert(tok);
  }
    

  fread((char*)&l,sizeof(int),1,file);

  for(x=0;x<l;x++){
    tok=new Token;
    tok->read(file);
    originalEdges.insert(tok);
  }


  fread((char*)&l,sizeof(int),1,file);

  originalModels.reset();
  for(x=0;x<l;x++){
    m=new int;
    fread((char*)m,sizeof(int),1,file);
    originalModels.insert(m);
  }

  
}




int
NetInfoType::getOriginals(int m,Token** tok,Token** etok){
  int *i;

  originalModels.reset();
  originalNodes.reset();
  originalEdges.reset();
  
  while(i=(int*) originalModels.getnext()){
    *tok=(Token*) originalNodes.getnext();
    *etok=(Token*) originalEdges.getnext();
    if(*i==m) break;
  }

  if(i==NULL) return 0;

  return 1;

}

