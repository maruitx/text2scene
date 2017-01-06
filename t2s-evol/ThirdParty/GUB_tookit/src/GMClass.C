/********************************************************************
              Copyright 1995 University of Bern              
     
                   All Rights Reserved
 
Permission to use this software and its documentation for any purpose
 and without fee is hereby granted, provided that the above copyright 
notice appear in all copies and that both that copyright notice and this 
permission notice appear in supporting documentation. Do not distribute 
without specific, written prior permission.  
                                                  
Author: Bruno T. Messmer	                       
Email:  messmer@iam.unibe.ch	                   
********************************************************************/

#include "GMClass.h"


AttributeClass ATT_object;
AttributeClass &ATT=ATT_object;

int STOP_BIT=0;



void signal_answer(int signum){
  fprintf(stderr,"Received user defined signal %d\n",signum);
  fprintf(stderr,"Stopping current process\n");
  STOP_BIT=1;
#ifdef SOLARIS
  signal(SIGALRM,signal_answer);
#endif
}



KB::KB(){

  Default_MODES[0]=1;
  Default_MODES[1]=2;
  Default_MODES[2]=1;
  Default_MODES[3]=2;
  Default_MODES[4]=1;
  Default_MODES[5]=3;

  


  modelNumber=0;
  mode=NORMAL;
  internal_status=0;

  graphs_owned_locally=1;

//#ifdef SOLARIS
//  signal(SIGALRM,signal_answer);
//#else
//#ifdef GCC
//  signal(SIGALRM,(SIG_PF) signal_answer);
//#else
//  signal(SIGALRM,signal_answer,-1);
//#endif
//#endif


  const char* name = "./Paras/";
  char* dname;
  
  //name=getenv("GM_HOME");
  //name = std::string("./").c_str();

  if(name==NULL){
    name=getenv("HOME");
    if(name==NULL){
      printf("Undefined ENV Variable HOME or GM_HOME ->  exiting...\n");
      exit(10);
    }
  }
  

  char dir[128];
  FILE* file;

  strcpy(dir,name);
  strcat(dir,"/.gm_defaults");
  initFromFile(dir);
 
  strcpy(dir,name); 
  strcat(dir,"/.gm_default_attributes");
  file=fopen(dir,"r");
  if(!file){
    printf("File %s is missing! -> exiting...\n",dir);
    exit(10);
  }
  fclose(file);  
  strcpy(DEFINITION_NAME,dir);
  
  Definition_flag=1;
  N.initAttributes(&ATT);  
   
}

KB::~KB(){
  discard();
}

int 
KB::status(){
  return internal_status;
}


void
KB::initFromFile(char* name) {

  FILE* file=fopen(name,"r");

  char buf[256];
  char* text;

  fgets(buf,255,file);

  strtok(buf,":");
  if(!(text=strtok('\0',". \n"))) return;

  int m=atoi(text);

  if(!(text=strtok('\0',". \n"))) return;

  int v=atoi(text);

  text=strtok('\0',". \n");
  double d=0;

  if(text)
    d=atof(text);

  setMethod(m,v,d);

  fclose(file);
  
}


void 
KB::setMethod(int method, int variante, double pars) {

  MatchingMethod=method;
  MatchingVariante=variante;
  MatchingPars=pars;

  discard();

  cout << " Graph-Matching Method is ";

  if(MatchingMethod==SECR) {

    

    if(variante==1) {
      cout << "exact ";
      p[0]=1;
      p[1]=2;
      p[2]=1;
      p[3]=2;
      p[4]=2;
      p[5]=1;
      p[6]=1;
	
    }else{
      cout << "error-correcting ";
      p[0]=1;
      p[1]=2;
      p[2]=1;
      p[3]=2;
      p[4]=1;
      p[5]=3;
      p[6]=0;
	
    }
    cout << "Network-Based Matching\n";
    N.setParameters(p);
    
    


  }else if(MatchingMethod==TREE) {
    if(variante==1) {
      cout << "Ullman's Method";
      p[0]=0;
      p[1]=2;
      p[2]=1;
      p[3]=2;
      p[4]=1;
      p[5]=1;
      p[6]=1;
    }else {
     cout << "A* ";
      p[0]=0;
      p[1]=1;
      p[2]=1;
      p[3]=2;
      p[4]=1;
      p[5]=1;
      p[6]=1;
    }
    if(variante==3) {
      cout << " with lookahead";
      p[5]=2;
    }
     
    cout << "\n";
    T.setParameters(p);

    
  }else if(MatchingMethod==POLY) {

    cout << "Polynomial Decision Tree Matcher\n";

    P.setMethod(variante, pars);

   

  }

 
}



void
KB::setKeyOfModel(int x,int key){

  ModelData[x].key=key;

}


/********* NICE INTERFACE *******/

void 
KB::addModel(Graph *G,int key){
  ModelData[modelNumber].G=G;
  ModelData[modelNumber].key=key;
  Modelgraph_flag=1;
 
  if(!ATT.rereadDefinition(DEFINITION_NAME)){
    ErrorMsg(std::string("Could not reread Definition file").c_str(),1);
    strcpy(DEFINITION_NAME,".gm_default_attributes");
	ErrorMsg(std::string("Trying default file .gm_defaut_attribute").c_str(), 1);
    if(!ATT.rereadDefinition(DEFINITION_NAME)){
		ErrorMsg(std::string("Could again not reread Definition file").c_str(), 1);
    }
  }

  //readParameterFile(SETTING_FILE);  
  if(MatchingMethod==SECR){
    
    N.clearNet();
    if(ModelData[modelNumber].G->AM.isGrain) N.setGrainInNetwork();
    N.compile(&ModelData[modelNumber].G->AM,modelNumber);
    N.setModelDataBase(ModelData,modelNumber+1);
        
  }else if(MatchingMethod==POLY) {

    if(P.admissibility(ModelData[modelNumber].G)) {
      P.addModel(ModelData[modelNumber].G,modelNumber);
      List ML;
      P.auxilaryModels(ML);
      while(ML.count()){
	G=(Graph*) ML.remove(0);
	modelNumber++;
	ModelData[modelNumber].G=G;
	ModelData[modelNumber].key=key;
      }
    }

  }
  modelNumber++;
}
  

void
KB::addModelAndUpdate(Graph *G,int key){
  ModelData[modelNumber].G=G;
  ModelData[modelNumber].key=key;
  Modelgraph_flag=1;
   
  if(MatchingMethod!=SECR){
    N.setParameters(Default_MODES);
  }

  N.lateCompile(&ModelData[modelNumber].G->AM,modelNumber);
  N.setModelDataBase(ModelData,modelNumber+1);  
  modelNumber++;
  N.setParameters(p);
  
}



void
KB::addGraphs(List& g,List& k,int type){
  Graph* G;
  int *key;
  List* Ml;
  int msn;
  In_MetaGraph* M;

  g.reset();
  k.reset();
  
  msn=modelNumber;
  Modelgraph_flag=1;
  while(G=(Graph*) g.getnext()){
    key=(int*) k.getnext();
    ModelData[modelNumber].G=G;
    ModelData[modelNumber].key=*key;
    modelNumber++;
  }

  Ml=createMetaGraph(g,type);
  N.buildFromMetaGraph(Ml,msn);  

  while(Ml->count()>0){
    M=(In_MetaGraph*) Ml->remove(0);
    delete M;
  }
  delete Ml;
  N.setModelDataBase(ModelData,modelNumber);  
}
  


int 
KB::loadDatabase(char* dir){
   FILE* file;
   char name[256];
   int method,variante,x;
   double pars;

   strcpy(name,dir);
   strcat(name,"/project");

   file=fopen(name,"r");
   if(!file){
     ErrorMsg(std::string("Could not open project file").c_str(),1);
     return 0;
   }
   
   discard();


   fread((char*)&method ,sizeof(int),1,file);
   fread((char*)&variante ,sizeof(int),1,file);
   fread((char*)&pars ,sizeof(double),1,file);


   setMethod(method,variante,pars);

   fread((char*)DEFINITION_NAME,sizeof(char),255,file);
   fread((char*)&modelNumber,sizeof(int),1,file);
   fread((char*)ModelData,sizeof(ModelDataBaseType),modelNumber,file);
 

   fclose(file);
   

   for(x=0;x<modelNumber;x++){
    char ch[8];
    char name2[64];
    strcpy(name,dir);
    strcat(name,"/model");
    sprintf(ch,"%d",x);
    strcat(name,ch);
    ModelData[x].G=new Graph;
    ModelData[x].G->read(name);
    strcpy(name2,"model");
    strcat(name2,ch);
    ModelData[x].G->setName(name2);
  }
   
   Modelgraph_flag=1;
   Definition_flag=1;
   
   loadProject(dir);

   return 1;
 }
  

int 
KB::saveDatabase(char* dir){
  char name[256];
  FILE* file;
  int x;

  file=fopen(dir,"r");
  if(!file){

    char text[256];

    strcpy(text,"mkdir ");
    strcat(text,dir);
    system(text);
    file=fopen(dir,"r");
    if(!file){
      strcpy(text,"There is no and could not create either directory ");
      strcat(text,dir);
      ErrorMsg(text,1);
      return 0;
    }
  }

  strcpy(name,dir);
  strcat(name,"/project");

  file=fopen(name,"w");
  
  fwrite((char*)&MatchingMethod ,sizeof(int),1,file);
  fwrite((char*)&MatchingVariante ,sizeof(int),1,file);
  fwrite((char*)&MatchingPars ,sizeof(double),1,file);

  fwrite((char*)DEFINITION_NAME,sizeof(char),255,file);
  fwrite((char*)&modelNumber,sizeof(int),1,file);
  fwrite((char*)ModelData,sizeof(ModelDataBaseType),modelNumber,file);

  fclose(file);

  for(x=0;x<modelNumber;x++){
    char ch[8];
    strcpy(name,dir);
    strcat(name,"/model");
    sprintf(ch,"%d",x);
    strcat(name,ch);
    ModelData[x].G->write(name);
  }


  saveProject(dir);


  return 1;
}

  
Graph* 
KB::getModel(int key){
  int x;
  for(x=0;x<modelNumber;x++)
    if(ModelData[x].key==key) return ModelData[x].G;

  return NULL;
}
  

Graph* 
KB::operator[](int i){
  return ModelData[i].G;
}
  

int 
KB::numberOfModels(){
  return modelNumber;
}



/********* OLD INTERFACE ********/


void
KB::startNewProject(char* name){
  modelNumber=0;
  discard();
  if(!ATT.loadDefinition(name)){
    ErrorMsg(std::string("Could not open definition file").c_str(),1);
    Definition_flag=0;
  }else{
    strcpy(DEFINITION_NAME,name);
    Definition_flag=1;
    N.initAttributes(&ATT);
  }
}





void
KB::loadProject(char* dir){
  char name[256];
  FILE* file;
  int x;

  strcpy(name,dir);
  strcat(name,"/attributes");
    
  if(!ATT.read(name)) {
    ErrorMsg(std::string("Could not open Attribute file").c_str(),1);
    return;
  }
  

  if(MatchingMethod==SECR){
    strcpy(name,dir);
    strcat(name,"/network");
    
    
    file=fopen(name,"r");
    if(file){
      fclose(file);
      N.read(name);
    }else{
      ErrorMsg("Could not open Project Directory",1);
      return;
    }

    N.initAttributes(&ATT);
    N.setModelDataBase(ModelData,modelNumber);

  }else if(MatchingMethod==POLY){

    strcpy(name,dir);
    strcat(name,"/decision");
    
    
    file=fopen(name,"r");
    if(file){
      fclose(file);
      P.saveNamesOfModels=0;
      P.read(name);
      P.setModelDataBase(ModelData,modelNumber);
    }else{
      ErrorMsg("Could not open Project Directory",1);
      return;
    }

  }
  ownsGraph(1);

  //readParameterFile(SETTING_FILE);

#if 0
  loadModels();
#endif
}



void
KB::saveProject(char* dir){
  char name[256];
  FILE* file;
  int x;

  
  strcpy(name,dir);
  strcat(name,"/attributes");
  
  ATT.write(name);


  if(MatchingMethod==SECR){
  
    strcpy(name,dir);
    strcat(name,"/network");
    
    N.write(name);
  }else if(MatchingMethod==POLY){
    strcpy(name,dir);
    strcat(name,"/decision");
    
    P.write(name);

  }

}


void
KB::clear(){
 int x;
 
 N.clearNet();
 N.discard();
 T.clear();
 P.discard();

 if(graphs_owned_locally==1){
   for(x=0;x<modelNumber;x++)
     delete ModelData[x].G;
 }
 modelNumber=0;
}


void
KB::discard(){
  int x;
 
  N.clearNet();
  N.discard();
  P.discard();

  T.clear();

  //ATT.discard();

  if(graphs_owned_locally==1){
    for(x=0;x<modelNumber;x++)
      delete ModelData[x].G;
  }

  modelNumber=0;
}



AttributeClass*
KB:: getAttributeObject(){
  return &ATT;
}

 

//void 
//KB::setTimerForEstimation(Timer* Tx,Series_Type *Et){
//
//  N.setTimerForEstimation(Tx,Et);
//
//}

void
KB::addModel(char* name){
  
  if(!ATT.rereadDefinition(DEFINITION_NAME))
	  ErrorMsg(std::string("Could not reread Definition file").c_str(), 1);
  Modelgraph_flag=1;
  ModelData[modelNumber].G=new Graph;
  if(!ModelData[modelNumber].G->AM.readFromFile(&ATT,name)){
    char text[256];
    strcpy(text,"Could not open model ");
    strcat(text,name);
    ErrorMsg(text,1);
    delete ModelData[modelNumber].G;
    ModelData[modelNumber].G=NULL;
    Modelgraph_flag=0;
    return;
  
  }
  ModelData[modelNumber].G->defaultKeys();
  ModelData[modelNumber].G->setName(name);
  ModelData[modelNumber].key=modelNumber;

  // readParameterFile(SETTING_FILE);  
  if(MatchingMethod==SECR){
    
    if(MatchingMethod!=SECR)
      N.setParameters(Default_MODES);
    N.clearNet();
    if(ModelData[modelNumber].G->AM.isGrain) N.setGrainInNetwork();
    N.setModelDataBase(ModelData,modelNumber+1);
    N.compile(&ModelData[modelNumber].G->AM,modelNumber);
    N.setParameters(p);
    

  }else if(MatchingMethod==POLY){

   if(P.admissibility(ModelData[modelNumber].G)) {
      P.addModel(ModelData[modelNumber].G,modelNumber);
    }

  }
  modelNumber++;
}
 

void
KB::displayModelBase(){
  int x;

  cout << "Model Base Contents:" << modelNumber << " Entries \n\n";
  for(x=0;x<modelNumber;x++){
    cout <<  ModelData[x].G->Name();
    cout << "   Nr: " << x << "\n";
  }
  
}
 

void 
KB::recognition(Graph *I){
  recognition(I,-1);
}

void 
KB::recognition(Graph *I,double threshold){

  if(!ATT.rereadDefinition(DEFINITION_NAME))
	  ErrorMsg(std::string("Could not reread Definition file").c_str(), 1);

  cout << " Graph-Matching Method is ";
  if(MatchingMethod==SECR) {
    if(MatchingVariante==1) {
      cout << "exact ";
    }else{
      cout << "error-correcting ";	
    }
    cout << "Network-Based Matching\n";     
  }else if(MatchingMethod==TREE) {
    if(MatchingVariante==1) {
      cout << "Ullman's Method"; 
    }else {
     cout << "A* ";
    }
    if(MatchingVariante==3) {
      cout << " with lookahead";  
    }    
    cout << "\n";   
  }else if(MatchingMethod==POLY) {
    cout << "Polynomial Decision Tree Matcher\n";
  }


  STOP_BIT=0;
  Image=I;
      
  if(MatchingMethod==SECR){
    N.initAttributes(&ATT);
    N.clearNet();
    internal_status=N.SECRrun(&Image->AM,threshold);

  }else if(MatchingMethod==TREE){
    //readParameterFile(SETTING_FILE); 
    T.setModelDataBase(ModelData,modelNumber);
    T.initAttributes(&ATT);
    T.match(&Image->AM,threshold);

  }else if(MatchingMethod==POLY) {

    P.recognition(Image);

  }
}





Graph* 
KB::recognition(char* name){

  /* until the first model is found */
  
  return recognition(name,-1);

}

// acient code, but it still works in *Matcher* and *Tenv*!
Graph*
KB::recognition(char* name,double threshold){
  
  if(!ConditionsTrue()) {
	  ErrorMsg(std::string("Cannot run recognition, check settings").c_str(), 1);
    return NULL;
  }

  if(!ATT.rereadDefinition(DEFINITION_NAME))
	  ErrorMsg(std::string("Could not reread Definition file").c_str(), 1);

  Image=new Graph;
  Image->AM.readFromFile(&ATT,name);
  Image->setName(name);
  Image->defaultKeys();
  
  recognition(Image,threshold);

  return Image;
}




void
KB::continueRecognition(){

  continueRecognition(-1);

}

void
KB::continueRecognition(double threshold){

  if(!ConditionsTrue()) {
	  ErrorMsg(std::string("Cannot run recognition, check settings").c_str(), 1);
    return;
  }
  STOP_BIT=0;

  if(MatchingMethod==SECR){
    
    internal_status=N.SECRcontinue(threshold);

  }else if(MatchingMethod==TREE){

    T.matchContinue(threshold);

  }
}


int 
KB::NumberOfMatches(){
    
    if(MatchingMethod==SECR){
      return N.NumberOfNewMatches();
    }else if(MatchingMethod==TREE){
      return T.NumberOfMatches();
    }else if(MatchingMethod==POLY) {
      return P.NumberOfMatches();
    }
}


int 
KB::NumberOfTotalMatches(){
  
     if(MatchingMethod==SECR){
       return N.NumberOfMatches();
     }else if(MatchingMethod==TREE){
       return T.NumberOfMatches();
     }else if(MatchingMethod==POLY) {
       return P.NumberOfMatches();
     }
}


InstanceData*
KB::getInstance(int i){
  InstanceData* ret;
  MappingData* MD;
  int x;
  Token* otok,*itok;

 
    if(MatchingMethod==SECR){
      MD=N.query(i);
    }else if(MatchingMethod==TREE){
      MD=T.query(i);
    }else if(MatchingMethod==POLY) {
      MD=P.query(i);
    }
    
    ret=new InstanceData;
    ret->numberOfSubstitutions=0;
    ret->modelNr=MD->getNumber();
    ret->modelKey=ModelData[ret->modelNr].key;
    otok=MD->Original();
    itok=MD->Image();
    ret->dim=otok->length();
    ret->model=new int[ret->dim];
    ret->image=new int[ret->dim];
    ret->error=new double[ret->dim];
    for(x=0;x<ret->dim;x++){
      int mv,iv;
      mv=ModelData[ret->modelNr].G->getI((*otok)[x]);
      if((*itok)[x]==-1)
	iv=-1;
      else{
	iv=Image->getI((*itok)[x]);
	ret->numberOfSubstitutions++;
      }
      ret->model[x]=mv;
      ret->image[x]=iv;
      ret->error[x]=itok->err(x);
    }
    ret->totalError=itok->totalErr();
    return ret;

  
}


InstanceData*
KB::getNewInstance(int i){
  InstanceData* ret;
  MappingData* MD;
  int x;
  Token* otok,*itok;

 
    if(MatchingMethod==SECR){
      MD=N.queryNew(i);
    }else if(MatchingMethod==TREE){
      MD=T.query(i);
    }else if(MatchingMethod==POLY) {
      MD=P.query(i);
    }

    ret=new InstanceData;
    ret->numberOfSubstitutions=0;
    ret->modelNr=MD->getNumber();
    ret->modelKey=ModelData[ret->modelNr].key;
    otok=MD->Original();
    itok=MD->Image();
    ret->dim=otok->length();
    ret->model=new int[ret->dim];
    ret->image=new int[ret->dim];
    ret->error=new double[ret->dim];
    for(x=0;x<ret->dim;x++){
      int mv,iv;
      mv=ModelData[ret->modelNr].G->getI((*otok)[x]);
      if((*itok)[x]==-1)
	iv=-1;
      else{
	iv=Image->getI((*itok)[x]);
	ret->numberOfSubstitutions++;
      }
      ret->model[x]=mv;
      ret->image[x]=iv;
      ret->error[x]=itok->err(x);
    }
    ret->totalError=itok->totalErr();
    return ret;
  
}


void
KB::removeInstances(InstanceData* ID){
  int n,x,i;
  int* vertices;

  if(MatchingMethod==SECR) {
    vertices=new int[ID->dim];
    n=ID->dim;
    i=0;
    for(x=0;x<n;x++){
      if(ID->image[x]!=-1)
	if(!Image->visible(ID->image[x]))
	  vertices[i++]=Image->getIndex(ID->image[x]);
    }    
    if(i>0)
      N.REMOVErun(vertices,i);
    
    delete vertices;
  }
}


int
KB::initQuery(int arg){

  if(!ConditionsTrue()) return 0;
  return N.initQuery(arg);
  
}


MappingData*
KB::getPartialMatchings(int *n,double *size){
  MappingData* mret;
  List Mlist;
  
  int x,num,d,i;
  Token* img,*org,*f_org,*f_img;

  if(!MatchingMethod==SECR) return NULL;
  
  *n=N.getPartialMatchings(&Mlist,size);
  
  if(*n==0) return NULL;

  mret=new MappingData[*n];
  x=0;
  while(Mlist.count()>0){
    MappingData* md;
    md=(MappingData*) Mlist.remove(0);

    num=md->getNumber();
    img=md->Image();
    org=md->Original();

    d=ModelData[num].G->numberOfVertices();
    f_org=new Token(d);
    f_img=new Token(d);
    
    for(i=0;i<d;i++){
      f_org->set(i,i);
      f_img->set(i,-1);
    }
    
    for(i=0;i<org->length();i++)
      f_img->set((*org)[i],(*img)[i]);


    mret[x].setNumber(num);
    mret[x].setMatch(f_img,f_org);
    delete md;
    delete f_org;
    delete f_img;
    x++;
  }
  return mret;

}


void
KB::displayAllInterpretation(){
 MappingData *MD;
  int x,y;
  
  if(MatchingMethod==SECR){
    
    y=N.NumberOfMatches();
    cout << "Matches: " << y << "\n";
    for(x=0;x<y;x++){
      
      MD=N.query(x);

      cout << "Name: " << ModelData[MD->getNumber()].G->Name() << " :\n";

      dump(MD);

      cout << "\n";
    }
  }else if(MatchingMethod==TREE){

    y=T.NumberOfMatches();
    cout << "TreeMatches: " << y << "\n";
    for(x=0;x<y;x++){
      
      MD=T.query(x);

      cout << "Name: " << ModelData[MD->getNumber()].G->Name() << " :\n";
      
      dump(MD);

      cout << "\n";
    }
  }else if(MatchingMethod==POLY){
     y=P.NumberOfMatches();
    cout << "Polynomial Matches: " << y << "\n";
    for(x=0;x<y;x++){
      
      MD=P.query(x);

      cout << "Name: " << ModelData[MD->getNumber()].G->Name() << " :\n";
      
      dump(MD);

      cout << "\n";
    }
  }
}


void 
KB::displayInterpretation(){
  MappingData *MD;
  int x,y;
  
 
    if(MatchingMethod==SECR){
      
      y=N.NumberOfNewMatches();
      cout << "Matches: " << y << "\n";
      for(x=0;x<y;x++){
	
	MD=N.queryNew(x);

	cout << "Name: " << ModelData[MD->getNumber()].G->Name() << " :\n";
	
	dump(MD);

	cout << "\n";
      }
    }else if(MatchingMethod==TREE){
      
      y=T.NumberOfMatches();
      cout << "TreeMatches: " << y << "\n";
      for(x=0;x<y;x++){
	
	MD=T.query(x);
	
	cout << "Name: " << ModelData[MD->getNumber()].G->Name() << " :\n";
	
	dump(MD);
	
	cout << "\n";
	
      }
    }else if(MatchingMethod==POLY) {
      y=P.NumberOfMatches();
      cout << "Polynomial Matches: " << y << "\n";
      for(x=0;x<y;x++){
	
	MD=P.query(x);
	
	cout << "Name: " << ModelData[MD->getNumber()].G->Name() << " :\n";
	
	dump(MD);
	
	cout << "\n";
	
      }
    }
}



void
KB::ErrorMsg(const char* text,int pr){

  fprintf(stderr,"Error in Matcher: %s\n",text);
  if(pr>3) 
    fprintf(stderr,"                  Fatal, abort program\n");
  else
    fprintf(stderr,"                  Continue\n");

}

int
KB::ConditionsTrue(){

  if(Modelgraph_flag&&Definition_flag) return 1;
  else return 0;

}


void
KB::dump(InstanceData* ID,char* nfile){
  Token *original,*model;
  int number,v,x,y;
  AttId mid,oid;
  Edit_Desc* eds;
  FILE* file;
  MappingData MD;

  if(nfile!=NULL){
    file=freopen(nfile,"w",stdout);
  }

  number=ID->modelNr;
  // extract a index oriented token array
  SortedList IndexList,IndexListImage;
  int d;

  d=ID->dim;
  for(x=0;x<ModelData[number].G->numberOfVertices();x++){
    v=ModelData[number].G->getI(x);
    IndexList.insert(NULL,x,v);
  }    
  for(x=0;x<Image->numberOfVertices();x++){
    v=Image->getI(x);
    IndexListImage.insert(NULL,x,v);
  }
  
  original=new Token(d);
  model=new Token(d);

  for(x=0;x<d;x++){
    v=IndexList.getValueKey(ID->model[x]);
    original->set(x,v);
    v=IndexListImage.getValueKey(ID->image[x]);
    model->set(x,v);
  }
 
  // fill SUB,INS,DEL list by calling totalError

  MD.setMatch(model,original);
  totalError(&ModelData[number].G->AM,&Image->AM,&MD);

  //

    
  for(y=0;y<original->length();y++){
    
    oid=ModelData[number].G->AM.getNodeAttributeId((*original)[y]);
    if((*model)[y]>-1)
      mid=Image->AM.getNodeAttributeId((*model)[y]);
    else
      mid.set(-1,NULL,0);

    printf(" %d - %d ",ID->model[y],ID->image[y]);

    if(mid.label()>-1){

//      printf(" %d - %d ",ATT.label(oid),ATT.label(mid));
      printf(" %d - %d ",oid.label(),mid.label());
    
      if(oid.label()==mid.label()){
	
	AttId o_array,m_array;
	int n;
	o_array=oid;
	m_array=mid;

	for(x=0;x<m_array.n;x++){
	  printf(" %5.2lf=%5.2lf",o_array[x],m_array[x]);
	}
      }
    }else{
      
      printf(" %d - __ ",oid.label());
    }
    printf("\n");
  }
  if(model->totalErr()>0) cout << " \n    Error:" << model->totalErr();
  
  if(SUB_list.count()>0)
    printf("Edge Substitutions:\n");
  while(SUB_list.count()>0){
    eds=(Edit_Desc*) SUB_list.remove(0);
    printf("%d to %d cost: %8.2lf \n",eds->edge_o ,eds->edge_i , eds->err );
    delete eds;
  }

  if(DEL_list.count()>0)
    printf("Edge Deletions:\n");
 
  int a1,a2,al1,ad;
  double *vals;
  while(DEL_list.count()>0){
    eds=(Edit_Desc*) DEL_list.remove(0);
    ModelData[number].G->getEdgeId(eds->edge_o,a1,a2,al1,vals,ad);
    printf("%d out: %d in: %d cost:  %8.2lf \n",eds->edge_o , a1, a2, eds->err);
    delete eds;
  }

   if(INS_list.count()>0)
    printf("Edge Insertions:\n");
  while(INS_list.count()>0){
    eds=(Edit_Desc*) INS_list.remove(0);
    printf("%d cost: %8.2lf \n",eds->edge_i , eds->err);
    delete eds;
  }  

  if(nfile!=NULL){
   
    freopen("/dev/tty","w",stdout);
    fclose(file);
  }


  IndexListImage.clear();
  IndexList.clear();

}


void
KB::dump(MappingData* MD){
  int number,y,x;
  AttId oid,mid;
  Token* original,*model;
  Edit_Desc* eds;
  
  number=MD->getNumber();
  
  
  cout << "ModelNr:" << number << " : \n"; 
  cout << " ModelIndex - ImageIndex   ModelLabel - ImageLabel  Att1 = Att2 .. \n";
  
  original=MD->Original();
  model=MD->Image();
  
  for(y=0;y<original->length();y++){
    
    oid=ModelData[number].G->AM.getNodeAttributeId((*original)[y]);
    if((*model)[y]>-1)
      mid=Image->AM.getNodeAttributeId((*model)[y]);
    else
#ifdef NEW_ATTOBJECT
      mid.set(-1,NULL,0);
#else
    mid=-1;
#endif
    
    cout << ""; 
    cout << (*original)[y];
    cout << "-";
    cout << (*model)[y];
    cout << "  ";
    
#ifdef NEW_ATTOBJECT
    if(mid.label()>-1){
#else
    if(mid>-1){
#endif
      cout << oid.label() << "-" << mid.label() << "  ";
      
      if(oid.label()==mid.label()){
	
#ifdef NEW_ATTOBJECT
	AttId o_array,m_array;
	int n;
	o_array=oid;
	m_array=mid;
#else	
	double* o_array,*m_array;
	int n;
	o_array=ATT.valueArray(oid,&n);
	m_array=ATT.valueArray(mid,&n);
#endif	
	for(x=0;x<m_array.n;x++){
	  cout << o_array[x] << "=" << m_array[x] << " ";
	}
      }
    }else{
      
      cout << oid.label() << "-  __"  << "  ";
    }
    cout << "\n";
  }
  if(model->totalErr()>0) cout << " \n    Error:" << model->totalErr() << "\n";
  
  if(SUB_list.count()>0)
    cout << "Edge Substitutions:\n";
  while(SUB_list.count()>0){
    eds=(Edit_Desc*) SUB_list.remove(0);
    cout << eds->edge_o << " to " << eds->edge_i << " cost: " << eds->err << "\n";
    delete eds;
  }
    
  if(DEL_list.count()>0)
      cout << "Edge Deletions:\n";
  int a1,a2,al1,ad;
  double *vals;
  while(DEL_list.count()>0){
    eds=(Edit_Desc*) DEL_list.remove(0);
    ModelData[number].G->getEdgeId(eds->edge_o,a1,a2,al1,vals,ad);
    printf("%d out: %d in: %d cost:  %lf \n",eds->edge_o , a1, a2, eds->err);
    delete eds;
  }

   if(INS_list.count()>0)
    cout << "Edge Insertions:\n";
  while(INS_list.count()>0){
    eds=(Edit_Desc*) INS_list.remove(0);
    cout << eds->edge_i <<  " cost: " << eds->err << "\n";
    delete eds;
  }  
    
}
  


void
KB::getDefaultParameters(int* &parms){
  int x;

   for(x=0;x<7;x++)
    parms[x]=Default_MODES[x];

}


void
KB::getParameters(int* &parms){
  int x;
  for(x=0;x<7;x++)
    parms[x]=p[x];
}
		  
  
void
KB::setParameters(int* parms){
  int x;
  
  for(x=0;x<7;x++)
    p[x]=parms[x];

  N.setParameters(p);
  T.setParameters(p);
 

  if((p[0]==1)||(p[0]==3))
    MatchingMethod=SECR;
  else
    MatchingMethod=TREE;

}


void
KB::readParameterFile(char* name,int* pp){
  char in[256];
  int x;
  FILE* file;
  char label,what;
  
  int i;

  
/***************************************************************/
// Specification in words: start file with `:'
//---------------------------------------------
// RETE , linear  , _ , mono    , inexact, pure   , -
//      , balanced,   , subgraph, exact  , static ,
//      ,         ,   , iso     ,        , dynamic,
//
// TREE , A*     ,best,         ,        , pure, -             
//      , Ullman ,depth         ,        , static, 
// Default: 1 1 1 1 1 1 
/***************************************************************/

  i=0;

  file=fopen(name,"r");

  if((file)&&(!feof(file))){
    what=getc_c(file);
    while(!feof(file)){
      switch(what){
      case '\n' : break;
      case '%' : 
	label=what;
	while((label!='\n')&&(!feof(file))){
	  label=getc_c(file);
	};
	break;
      case '.':
	pp[i]=(int) get_zahl(file);
	i++;
	break;
      case ':':
	interpretParameterfile(file,pp);
	break;
      }
      what=getc_c(file);
    }
    fclose(file);
  }else{
	  ErrorMsg(std::string("Using default Parameter settings: RETE balanced _ sub tolerant dynamic").c_str(), 1);
    pp[0]=Default_MODES[1];
    pp[1]=Default_MODES[2];
    pp[2]=Default_MODES[3];
    pp[3]=Default_MODES[4];
    pp[4]=Default_MODES[5];
    pp[5]=Default_MODES[6];
  
  }
 
  if((pp[0]==1)||(pp[0]==3))
    MatchingMethod=SECR;
  else
    MatchingMethod=TREE;
}


void
KB::interpretParameterfile(FILE* file,int* p){
  char terminal[6][5][20];
  int i,ii,z,x,y,match;
  char text[32],what,label;

  for(x=0;x<7;x++) p[x]=1;

  for(x=0;x<6;x++)
    for(y=0;y<4;y++) strcpy(terminal[x][y],"@");

  strcpy(terminal[0][0],"TREE");
  strcpy(terminal[0][1],"RETE");
  strcpy(terminal[1][0],"linear");
  strcpy(terminal[1][1],"balanced");
  strcpy(terminal[1][2],"A*");
  strcpy(terminal[1][3],"Ullmann");
  strcpy(terminal[1][4],"compressed");
  strcpy(terminal[2][0],"best");
  strcpy(terminal[2][1],"depth");
  strcpy(terminal[3][0],"monomorphism");
  strcpy(terminal[3][1],"subgraph");
  strcpy(terminal[3][2],"isomorphism");
  strcpy(terminal[4][0],"tolerant");
  strcpy(terminal[4][1],"exact");
  strcpy(terminal[5][0],"pure");
  strcpy(terminal[5][1],"static");
  strcpy(terminal[5][2],"dynamic");
  strcpy(terminal[5][3],"future");

  i=0;
  z=0;
  ii=0;
  match=0;

  what=getc_c(file);
    while(!feof(file)){
      switch(what){
      case '\n' : match=2; break;
      case '%' : 
	while((label!='\n')&&(!feof(file))){
	  label=getc_c(file);
	};
	match=2;
	break;
      case ' ': if(!match) match=1; break;
      default: text[ii]=what; ii++; match=0;
      }

      if(match>0){
	text[ii]='\0';
	ii=0;
	for(z=0;z<4;z++){
	  if(terminal[i][z][0]=='@') {
	    p[i]=1;
	    break;
	  }
	  if(strstr(terminal[i][z],text)){
	    if(i>0) p[i]=z+1;
	    else p[i]=z;
	    if(i==5){
	      if(z==3) p[i]=0;
	    }
	    break;
	  }
	}
	i++;
      }
      if(i>6) return;
      if(match>1) return;

      what=fgetc(file);
    }

}


void
KB::setMatchingMethod(int m){
  
  MatchingMethod=m;
}


void
KB::prepareFromBase(int* pset){
  int x;

  gmt_assert(0);

  N.clearNet();
  N.discard();

  if(pset==NULL){
    readParameterFile(SETTING_FILE,p);  
    T.setParameters(p);
    N.setParameters(p);    
pset=p;
  }else{
    if(MatchingMethod==SECR) N.setParameters(pset);
    else T.setParameters(pset);
   
  }

  if((MatchingMethod==SECR)||(1)){

    if(pset[1]>=3){
      List g,k;
      for(x=0;x<modelNumber;x++){
	g.insert(&ModelData[x].G->AM);
	k.insert(&ModelData[x].key);
      }
      modelNumber=0;
      addGraphs(g,k,pset[1]);

    }else{
      for(x=0;x<modelNumber;x++){
	
	if(ModelData[x].G->AM.isGrain){
	  if(pset[6]==0) continue;
	  N.setGrainInNetwork();
	}
	
	N.compile(&ModelData[x].G->AM,x);
    
      }
      N.setModelDataBase(ModelData,modelNumber);
    }
  

  }
}




double
KB::totalError(AdjazenzMatrix *O,AdjazenzMatrix *I,MappingData* MD){
  int x,y,l,r;
  AttId oat,at;
  int edg,g;
  double d,sumErr,err;
  Token *otok,*itok;
  int *path;
  AttId e_l[32],e_r[32];
  int edge_l[32],edge_r[32];

  

  if(p[3]==1) ATT.setInsertionCost(0);

  otok=MD->Original();
  itok=MD->Image();

  sumErr=0;

/* Vertex Label error caculation  */

  for(x=0;x<otok->length();x++){
    oat=O->getNodeAttributeId((*otok)[x]);

    if((*itok)[x]==-1){
      err=ATT.deletionCost(oat);
    }else{
      at=I->getNodeAttributeId((*itok)[x]);    
      err=ATT.error(oat,at);
    }
    sumErr+=err;
  }

  cout << "Node Error: " << sumErr << "   ";
  
/* Edge Label error calculation  */

  d=0;
  for(x=0;x<otok->length();x++){
    for(y=x;y<otok->length();y++){

      double **m;
     
      int dim_l,dim_r;
      int i,j,z;

      l=(*otok)[x];
      r=(*otok)[y];
      
      for(z=0;z<2;z++){
	
	if((z==1)&&(x==y)) continue;
	if(z==0)
	  O->initNext(l,r);
	else
	  O->initNext(r,l);
       
	dim_l=0;dim_r=0;
	
	while((oat=O->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	  e_l[dim_l]=oat;
	  edge_l[dim_l++]=edg;
	  
	}
#ifdef NEW_ATTOBJECT
	edge_l[dim_l]=-1;
	e_l[dim_l++].set(-1,NULL,0);
	
#else
	e_l[dim_l++]=-1;
    
#endif	
	if(((*itok)[x]<0)||((*itok)[y]<0)){
	  for(i=0;i<dim_l-1;i++){
	    Edit_Desc *eds;
	    eds=new Edit_Desc;
	    eds->edge_o=edge_l[i];
	    eds->err=ATT.deletionCostOfEdge(e_l[i],(*itok)[x],(*itok)[y],(Graph*) I->owner);
	    d+=eds->err;
	    DEL_list.insert(eds);
	  }
	}else{

	  if(z==0)
	    I->initNext((*itok)[x],(*itok)[y]);
	  else
	    I->initNext((*itok)[y],(*itok)[x]);
	  
	  while((at=I->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	    e_r[dim_r]=at;
	    edge_r[dim_r++]=edg;
	  }
#ifdef NEW_ATTOBJECT
	  edge_r[dim_r]=-1;
	  e_r[dim_r++].set(-1,NULL,0);
#else
	  e_r[dim_r++]=-1;
#endif	  
	  if(dim_l*dim_r>1){
	    
	    m=new double*[dim_l];
	    for(i=0;i<dim_l;i++) m[i]=new double[dim_r];
	    
	    for(i=0;i<dim_l;i++)
	      for(j=0;j<dim_r;j++){
		if(i==dim_l-1){
		  if(j<dim_r-1)
		    m[i][j]=ATT.insertionCostOfEdge(e_r[j],1,1,(Graph*) O->owner);
		}else{
		  if(j<dim_r-1)
		    m[i][j]=ATT.error(e_l[i],e_r[j]);
		  else{
		    if(z==0)
		      m[i][j]=ATT.deletionCostOfEdge(e_l[i],(*itok)[x],(*itok)[y],(Graph*) I->owner);
		    else
		      m[i][j]=ATT.deletionCostOfEdge(e_l[i],(*itok)[y],(*itok)[x],(Graph*) I->owner);
		  }
		} 
	      }
	    
	    path=new int[dim_l];
	    d+=HS_smart(m,dim_l,dim_r,path);
	    gmt_assert(d<INFTY_COST);
	    collect_edge_edits(dim_l,dim_r,path,edge_l,edge_r,m,SUB_list,DEL_list,INS_list);

	    for(i=0;i<dim_l;i++) delete m[i];
	    delete m;
	    delete path;
	  }
	}   
	if(!I->isDirected()) break;
      }
    }
  }

  cout << "Edge Error (Mono): " << d << "\n";
  sumErr+=d;

  cout << "Total: " << sumErr << "\n";


  ATT.setInsertionCost(1);
  return sumErr;


}


void
KB::getStatistics(int* expansion,int* memory,long* time,int* instances,double* minError,int* checks){

  
    if(MatchingMethod==SECR)
      N.getStatistics(expansion,memory,time,instances,minError,checks);
    else if(MatchingMethod==TREE)
      T.getStatistics(expansion,memory,time,instances,minError,checks);
    else if(MatchingMethod==POLY)
      P.getStatistics(expansion,memory,time,instances,minError,checks);
  
}


int*
KB::distributionPartials(int& count){

  if(MatchingMethod==SECR)
    return N.distributionPartials(count);
  else if(MatchingMethod==TREE)
    return T.distributionPartials(count);

  return NULL;
}


void
KB::ownsGraph(int i){
  graphs_owned_locally=i;
}


void
KB::getNetworkStructure(int* iarray,int& n){
  int x,i;

  iarray[0]=N.NNodeX+N.CNodeX+N.MNodeX;
  iarray[1]=N.NNodeX;
  iarray[2]=N.CNodeX;
  iarray[3]=N.MNodeX;
  iarray[4]=N.NetworkDepth;
  
  int v=0;
  int e=0;
  for(x=0;x<modelNumber;x++){
    v+=ModelData[x].G->numberOfVertices();
    e+=ModelData[x].G->numberOfEdges();
  }

  iarray[5]=v;
  iarray[6]=e;
  iarray[7]=N.NNodeX;

  v=0;
  for(x=0;x<N.NNodeX;x++)
    v+=N.NNodes[x].testList.count();

   for(x=0;x<N.CNodeX;x++)
    v+=N.CNodes[x].testList.count();

  iarray[8]=v;
  

}


List*
KB::getNetworkData(){
  List *data;
  List *nlist;
  GeneralInfoStruct* gni;

  data= N.getNetworkData();

  data->reset();
  while(nlist=(List*) data->getnext()){
    nlist->reset();
    while(gni=(GeneralInfoStruct*) nlist->getnext()){
      gni->key=ModelData[gni->key].key;
    }
  }

  return data;
}


void
KB::setImage(Graph* G){
  Image=G;
 
}

void
KB::setUpperThreshold(double ut){
  N.cut_threshold=ut;
  T.cut_threshold=ut;
}







char*
KB::getAttributeFilename(){
  return DEFINITION_NAME;
}
