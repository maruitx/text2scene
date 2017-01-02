#include "gmt_assert.h"
#include <stdio.h>
//#include "AttributeClass.h"
#include "Graph.h"


 const int SUB_FUNCTION=1;
 const int INS_OF_EDGES=2;
 const int INS_OF_VERTICES=3;
 const int DEL_OF_EDGES=4;
 const int DEL_OF_VERTICES=5;
 const int SUB_COMPLEX=6;
 const int INS_OF_COMPLEX_EDGES=7;
 const int INS_OF_COMPLEX_VERTICES=8;
 const int DEL_OF_COMPLEX_EDGES=9;
 const int DEL_OF_COMPLEX_VERTICES=10;
 const int SUB_FUNCTION_COMPLEX=11;

 const int DESCRIPTOR_GRAPH=-10;
static int ENV_COST_QUADRATIC=0;

double (*substitution_cost_function)(int label1,double *values1,int n1,
					    double *w1,double *t1,
					    int label2,double *values2,int n2,
					    double *w2,double *t2,
					    double lw1,double lw2)=NULL;

double (*vertex_insertion_cost_function)(int label1,double* values1,int n1,double ins_error)=NULL;
double (*vertex_deletion_cost_function)(int label1,double* values1,int n1,double del_error)=NULL;
double (*edge_insertion_cost_function)(int label1,double* values1,int n1,int f1,int f2,double ins_error,Graph* G)=NULL;
double (*edge_deletion_cost_function)(int label1,double* values1,int n1,int f1,int f2,double del_error,Graph* G)=NULL;


double (*substitution_cost_complex)(int label1,void* complex1, int descr1,
					   int label2,void* complex2, int descr2) = NULL;
double (*vertex_insertion_cost_complex)(int label1,void* complex,int descr, double ins_error)=NULL;
double (*vertex_deletion_cost_complex)(int label1, void* complex,int descr,double del_error)=NULL;
double (*edge_insertion_cost_complex)(int label1, void* complex,int descr,int f1,int f2,double ins_error,Graph* G)=NULL;
double (*edge_deletion_cost_complex)(int label1, void* complex,int descr,int f1,int f2,double del_error,Graph* G)=NULL;

double (*substitution_cost_function_complex)(int label1,double *values1,int n1,
				     double *w1,double *t1,
				     int label2,double *values2,int n2,
				     double *w2,double *t2,
				     double lw1,double lw2,
				     void *complex1, int desc1,
				     void *complex2, int desc2)=NULL;


void registerFunction(int typ,void* (*func)()){
  
  switch(typ){
  case SUB_FUNCTION: substitution_cost_function=(double (*)(int label1,double *values1,int n1,
							    double *w1,double *t1,
							    int label2,double *values2,int n2,
							    double *w2,double *t2,
							    double lw1,double lw2)) func;
							    break;
  case INS_OF_VERTICES: vertex_insertion_cost_function=(double (*)(int label1,double* values1,
								   int n1,double ins_error)) func;
							    break;
  case DEL_OF_VERTICES: vertex_deletion_cost_function=(double (*)(int label1,double* values1,
								  int n1,double del_error)) func;
							    break;
  case INS_OF_EDGES: edge_insertion_cost_function=(double (*)(int label1,double* values1,int n1,
							      int f1,int f2,double ins_error,Graph* G)) func;
							    break;
  case DEL_OF_EDGES: edge_deletion_cost_function=(double (*)(int label1,double* values1,int n1,
							     int f1,int f2,double del_error,Graph* G)) func;
							    break;
	
  case SUB_COMPLEX: substitution_cost_complex=(double (*)(int label1,void* complex1, int descr1, int label2,void* complex2, int descr2)) func;
							    break;
  case INS_OF_COMPLEX_VERTICES: vertex_insertion_cost_complex=(double (*)(int label1,void* complex,int descr, double ins_error)) func;
							    break;
  case DEL_OF_COMPLEX_VERTICES: vertex_deletion_cost_complex=(double (*)(int label1, void* complex,int descr,double del_error)) func;
							    break;
  case INS_OF_COMPLEX_EDGES: edge_insertion_cost_complex=(double (*)(int label1, void* complex,int descr,int f1,int f2,double ins_error,Graph* G)) func;
							    break;
  case DEL_OF_COMPLEX_EDGES: edge_deletion_cost_complex=(double (*)(int label1, void* complex,int descr,int f1,int f2,double del_error,Graph* G)) func;
							    break;
				
  case SUB_FUNCTION_COMPLEX: substitution_cost_function_complex=(double (*)(int label1,double *values1,int n1,
									    double *w1,double *t1,
									    int label2,double *values2,int n2,
									    double *w2,double *t2,
									    double lw1,double lw2,
									    void* complex1, int desc1,
									    void* complex2, int desc2)) func;
							    break;
							    
			    
							  }
  
}



List GARBAGE;

AttributeClass::AttributeClass(int maxLab){
  int x,y;

  error_checks = 0;

 INSERTION_COST=1;
 DELETION_COST=1;
  LabelGroup = new AttType[maxLab];
  maxLabels=maxLab;
  NumberOfRefs=0;
  NumberOfLabels=0;

  CTable = new MatrixEntry*[maxLabels];
  for(x=0;x<maxLabels;x++){
    CTable[x]=new MatrixEntry[maxLabels];
    for(y=0;y<maxLabels;y++) {
      CTable[x][y].weight=-2;
      CTable[x][y].vweights=NULL;
    }
  }
  initialized=0;

}


AttributeClass::AttributeClass(){
  int x,y;
 
  error_checks = 0;

  INSERTION_COST=1;
  DELETION_COST=1;
  maxLabels=MAX_LABELS;
  LabelGroup=new AttType[maxLabels];
  NumberOfRefs=0;
  NumberOfLabels=0;
  REREAD=0;
  initialized=0;
  CTable = new MatrixEntry*[maxLabels];
  for(x=0;x<maxLabels;x++){
    CTable[x]=new MatrixEntry[maxLabels];
    for(y=0;y<maxLabels;y++) {
      CTable[x][y].weight=-2;
      CTable[x][y].vweights=NULL;
    }
  }

  const char* name = "./Paras/";
  char def_name[128];
  


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
  strcat(dir,"/.gm_default_attributes");
  file=fopen(dir,"r");
  if(!file){
    printf("File %s is missing! -> exiting...\n",dir);
    exit(10);
  }
  fclose(file);  
  strcpy(def_name,dir);
  
  name=getenv("NM_DEFINITION_FILE");
  if(name)  
    strcpy(def_name,name);
  
  loadDefinition(def_name);

  name=getenv("ENV_COST_QUADRATIC");
  if(name) ENV_COST_QUADRATIC=1;

}

void
AttributeClass::init(){
  int x,y;
  
  INSERTION_COST=1;
  DELETION_COST=1;
  maxLabels=MAX_LABELS;
  LabelGroup=new AttType[maxLabels];
  NumberOfRefs=0;
  NumberOfLabels=0;
  REREAD=0;
  initialized=0;
  CTable = new MatrixEntry*[maxLabels];
  for(x=0;x<maxLabels;x++){
    CTable[x]=new MatrixEntry[maxLabels];
    for(y=0;y<maxLabels;y++) {
      CTable[x][y].weight=-2;
      CTable[x][y].vweights=NULL;
    }
  }

  const char* name;
  char def_name[128];
  
  //name=getenv("GM_HOME");
  name = std::string("./").c_str();

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
  strcat(dir,"/.gm_default_attributes");
  file=fopen(dir,"r");
  if(!file){
    printf("File %s is missing! -> exiting...\n",dir);
    exit(10);
  }
  fclose(file);  
  strcpy(def_name,dir);


  name=getenv("NM_DEFINITION_FILE");
  if(name){  
    strcpy(def_name,name);
  }
  loadDefinition(def_name);
  
}

AttributeClass::~AttributeClass(){
  int x;
  discard();
  delete[] LabelGroup;

  for(x=0;x<maxLabels;x++)
    delete CTable[x];

  delete[] CTable;
  garbageCollection();
}


void
AttributeClass::discard(){
  int x,y;
  for(x=0;x<NumberOfLabels;x++){
    LabelGroup[x].Values.discard();
    LabelGroup[x].dim=0;
    LabelGroup[x].insertionError=0;
    delete LabelGroup[x].weights;
    delete LabelGroup[x].thresholds;
  }

  

  for(x=0;x<maxLabels;x++){
     for(y=0;y<maxLabels;y++) {
      CTable[x][y].weight=-2;
      CTable[x][y].vweights=NULL;
    }
  }

  

  NumberOfLabels=0;
  NumberOfRefs=0;

}

void
AttributeClass::garbageCollection(){
    int i=0;
    if(GARBAGE.count()>0){
	cout << "Garbage Collection ...";
	while(GARBAGE.count()){
	    AttId* at=(AttId*) GARBAGE.remove(0);
	    i++;
	    delete at;
	}
	cout << i << " items found and destroyed\n"; 
    }   
}


int
AttributeClass::lookupLabel(int label){

  int x;
  gmt_assert(label>=0);
  if(label<NumberOfLabels)
    if(LabelGroup[label].label==label) return label;

  for(x=0;x<NumberOfLabels;x++){
    if(LabelGroup[x].label==label) return x;
  }

  return -1;


}


int
AttributeClass::randomLabel(AttId at,double err){

  int t,i,c;
  double e;
  
  if(at.label()<(NumberOfLabels/2)) c=1; else c=-1;
  i=0;
  while(i<NumberOfLabels){
    i=i+c;
    t=(at.label()+i) % (NumberOfLabels-1);
    if(t<0) t=t+NumberOfLabels;

    if(t==at.label()) continue;

    if(CTable[at.label()][t].weight==-2)
      e=CTable[t][at.label()].weight;
    else
      e=CTable[at.label()][t].weight;

    if((e>0)&&(e<=err)&&(e<insertionCost(at))) break;
  }

  return t;

}


AttId
AttributeClass::registerLabel(int label, double* values, int count){
  int ind;
  int c;
#if 0
  if((c=lookupLabel(label))<0){printf("FATAL: no such label!"); assert(0);}

  if(count!=LabelGroup[c].dim){printf("FATAL: ATT Count wrong!"); assert(0);};

  ind=LabelGroup[c].Values.add(values);


  NumberOfRefs++;
  
  if(NumberOfRefs>=MAX_ATTID) {printf("FATAL: MAX_ATTID<!"); assert(0);}
  
  REF[NumberOfRefs].group=c;
  REF[NumberOfRefs].index=ind;
  
  return NumberOfRefs;
#endif

  if((c=lookupLabel(label))<0){printf("FATAL: no such label!"); gmt_assert(0);}

  if(count!=LabelGroup[c].dim){printf("FATAL: ATT Count wrong!"); gmt_assert(0);};

  AttId *at;
  at=new AttId;
  at->set(label,values,count);


  GARBAGE.insert(at); 
 
  /** this instance of AttId is garbage as soon as the function has finished returning */
  /** however, due to the shallowcopy of the operator= we must preserve the values     */
  /** a little bit longer */
  /** destroy at some point in the future */
 
  return *at;
  
}



void
AttributeClass::refreshParameters(int label, double* weights, double* thresholds, double INS,double DEL,double cyc){
  int c,x;

   c=lookupLabel(label);

  if(c<0){
    printf("Current Attributes and Refresh do not match -> refresh cancelled\n");
    return ;
  }
  
  LabelGroup[c].cycleTop=cyc;
  
  for(x=0;x<LabelGroup[c].dim;x++){
    LabelGroup[c].weights[x]=weights[x];
    LabelGroup[c].thresholds[x]=thresholds[x];
  }

  LabelGroup[c].insertionError=INS;
  if(INS<0) INS=INFTY_COST;
  if(DEL<0) DEL=INFTY_COST;
  LabelGroup[c].insertionError=INS;
  LabelGroup[c].deletionError=DEL;

}


void
AttributeClass::define(int label, double* weights, double* thresholds,int count, double INS,double DEL, double cyc){
  int c,x;

  c=lookupLabel(label);

  if(c>=0) {printf("FATAL: label already defined!"); gmt_assert(0);}

  
  NumberOfLabels++;

  if(NumberOfLabels>=maxLabels) {printf("FATAL: too many labels!"); gmt_assert(0);};

  LabelGroup[NumberOfLabels-1].label=label;

  LabelGroup[NumberOfLabels-1].cycleTop=cyc;
  
  LabelGroup[NumberOfLabels-1].weights= new double[count];
  LabelGroup[NumberOfLabels-1].thresholds= new double[count];

  for(x=0;x<count;x++){
    LabelGroup[NumberOfLabels-1].weights[x]=weights[x];
    LabelGroup[NumberOfLabels-1].thresholds[x]=thresholds[x];
  }

  LabelGroup[NumberOfLabels-1].dim=count;
  LabelGroup[NumberOfLabels-1].Values.init(count);

  if(INS<0) INS=INFTY_COST;
  if(DEL<0) DEL=INFTY_COST;
  LabelGroup[NumberOfLabels-1].insertionError=INS;
  LabelGroup[NumberOfLabels-1].deletionError=DEL;

}


void
AttributeClass::defineCostTableEntry(int label1,int label2, double weight, double *rule,int dim){
  int x;

  if(CTable[label2][label1].weight<0)
    CTable[label2][label1].weight=-2;
  
  CTable[label1][label2].weight=weight;
  
  if(rule){
    CTable[label1][label2].vweights=new double[dim];
    for(x=0;x<dim;x++) CTable[label1][label2].vweights[x]=rule[x];
  }
  
}



int
AttributeClass::rereadDefinition(char* name){

  REREAD=1;
  if(!loadDefinition(name)) return 0;
  REREAD=0;
  return 1;
}


int
AttributeClass::loadDefFileFromGraph(char* name){
  FILE* file;
  char str[256];
  char dname[256];
  char* ptr;
  int i,END;
  char a;
  
  END=0;
  file = fopen(name,"r");

  if(!file) {printf("FATAL:could not open file!");gmt_assert(0);};

  a=getc_c(file);
  while((!feof(file))&&(END==0)){
    fgets(str,255,file);
    if(ptr=strstr(str,"DEFNAME:")){
      i=8;
      while((ptr[i]!='\n')&&(ptr[i]!=' ')) i++;
      strncpy(dname,&ptr[8],i-8);
      END=1;
    }
  }
   
  fclose(file);
  if(END){
    loadDefinition(dname);
  }
  return 1;
}


char*
AttributeClass::name(){
  return Name;
}



int 
AttributeClass::numberOfLabels(){
  return NumberOfLabels;
}



int
AttributeClass::loadDefinition(char* name){
  FILE *file;
  char text[256];
  char a;
  int dim,END,x,y,from,to,from2,to2,diff_flag;
  double w,ww,W[30],T[30];
  double INS,DEL;
  double cyc;
  char what;
  
  // printf("Reading attribute definition from file %s \n",name); 

  END=0;
  file = fopen(name,"r");

  if(!file) return 0;

  strcpy(Name,name);

  a=getc_c(file);
  if(a!='#') return 0;

  if(!REREAD) discard();

  while(getc_c(file)!='\n');

  while((!feof(file))&&(END==0)){

    fgets(text,255,file);
  
    if(text[0]!='=') continue;
    
    switch(text[1]){
    case 'L': 
      /*   =L=                                    */
      /*   l: 1-10 d:4 t:0.2    ... w:0.8 ..i:8...*/
      
      
      if(getc_c(file)=='l'){
	from=(int) get_zahl(file);
	to=from;
	if(getc_c(file)=='-'){
	  to=(int) get_zahl(file);
	}
      }else{return 0;};

      what=getc_c(file);

      cyc=0;
      if(what=='c'){
	cyc= get_zahl(file);
	what=getc_c(file);
      }

      if(what=='d'){
	dim=(int) get_zahl(file);

      }else{return 0;};

      if(getc_c(file)=='t'){
	for(x=0;x<dim;x++) T[x]=get_zahl(file);
      }
      
      if(getc_c(file)=='w'){
	for(x=0;x<dim;x++) W[x]=get_zahl(file);
      }

      if(getc_c(file)=='i') INS=get_zahl(file);
 
#ifdef DELETION_SPEC     
      if(getc_c(file)=='c') DEL=get_zahl(file);
#else
      DEL=INS;
#endif

      for(x=from;x<=to;x++){
	if(REREAD==0)
	  define(x,W,T,dim,INS,DEL,cyc);
	else
	  refreshParameters(x,W,T,INS,DEL,cyc);
      }

      break;
    case 'T':
      /*   =T=                                    */
      /*   1-10,2:  + 0.3  or  | 0.3   or ~      */

      from=(int) get_zahl(file);
      to=from;
      if(getc_c(file)=='-')
	to=(int) get_zahl(file);

      from2=(int) get_zahl(file);
      to2=from2;
      if(getc_c(file)=='-'){
	to2=(int) get_zahl(file);
	getc_c(file);
      }

      diff_flag=0;
      

      switch(getc_c(file)){
      case '|':
	diff_flag=1;
	w=get_zahl(file);
	break;
      case '+':
	diff_flag=0;
	w=get_zahl(file);
	break;
      case '~':
	diff_flag=-1;
	break;

      }

      for(x=from;x<=to;x++){
	for(y=from2;y<=to2;y++){
	  if(diff_flag){
	    ww=w*(y-x);
	    defineCostTableEntry(x,y,ww, NULL,0);
	  }else{
	    if(diff_flag==0)
	      defineCostTableEntry(x,y,w, NULL,0);
	    else
	      defineCostTableEntry(x,y,-1, NULL,0);
	  }
	}
      }
      break;

    case 'E': END=1;

      break;
    default:continue;
    }
  
    
  
  }


  fclose(file);
  if(!END) 
	return 0; 
  else{
    initialized=1;
    return 1;
  }
}

int
AttributeClass::defined(){
  return initialized;
}


/***   Retrieval   ****/



int 
AttributeClass::Label(AttId &id){
  return LabelGroup[REF[id.label()].group].label;
}


double* 
AttributeClass::valueArray(AttId id,int *n){
#if 0
  int l,ind;
  l=REF[id].group;
  ind=REF[id].index;
  *n=LabelGroup[l].dim;
  return LabelGroup[l].Values.array(ind);
#endif
  return 0;
}


double
AttributeClass::insertionCost(AttId &A1){
  int l;
  double err=0;

  if(INSERTION_COST==0)
    return 0.0;
  
  if(vertex_insertion_cost_complex!=NULL){
    
    err=vertex_insertion_cost_complex(A1.Label, A1.complex_value->data, A1.complex_value->descriptor, LabelGroup[l].insertionError);
    return err;
  }
  
  if(vertex_insertion_cost_function!=NULL){
    
    l=lookupLabel(A1.label());
    err=vertex_insertion_cost_function(A1.Label,A1.values,A1.n,LabelGroup[l].insertionError);
    return err;
  }else{
    if(INSERTION_COST==0){
      return 0.0;
    }else{
#ifndef NEW_ATTOBJECT
      l=REF[A1.label()].group;
#else
      l=lookupLabel(A1.label());
#endif
      
      return LabelGroup[l].insertionError;
      
    }
  }
}


double
AttributeClass::insertionCostOfEdge(AttId &A1,int n1,int n2,Graph* G){
  int l;
 

  if(INSERTION_COST==0)
   return 0.0;
  
  if(edge_insertion_cost_complex!=NULL){
    
    double err=edge_insertion_cost_complex(A1.Label, A1.complex_value->data, A1.complex_value->descriptor, n1,n2, LabelGroup[l].insertionError,G);
    return err;
  }
  

  if(edge_insertion_cost_function!=NULL){
    double err;
    l=lookupLabel(A1.label());
    err=edge_insertion_cost_function(A1.Label,A1.values,A1.n,n1,n2,LabelGroup[l].insertionError,G);
    return err;
  }else{
    
    if(INSERTION_COST==0){
      return 0.0;
    }else{
#ifndef NEW_ATTOBJECT
      l=REF[A1.label()].group;
#else
      l=lookupLabel(A1.label());
#endif  
      return LabelGroup[l].insertionError;
    }
  }
}


void
AttributeClass::setDeletionCost(int f){
  DELETION_COST=f;
}


void
AttributeClass::setInsertionCost(int f){
  INSERTION_COST=f;
}


double
AttributeClass::deletionCost(AttId &A1){
  int l;
  

  if(vertex_deletion_cost_complex!=NULL){
    double err;
    err=vertex_deletion_cost_complex(A1.Label, A1.complex_value->data, A1.complex_value->descriptor, LabelGroup[l].insertionError);
    return err;
  }

  if(vertex_deletion_cost_function!=NULL){
    double err;
    l=lookupLabel(A1.label());
    err=vertex_deletion_cost_function(A1.Label,A1.values,A1.n,LabelGroup[l].deletionError);
    return err;
  }else{
    
    
    if(DELETION_COST==0){
      return 0.0;
    }else{
#ifndef NEW_ATTOBJECT
      l=REF[A1.label()].group; 
#else
      l=lookupLabel(A1.label());
#endif
      return LabelGroup[l].deletionError;
    }
  }
}


double
AttributeClass::deletionCostOfEdge(AttId &A1,int n1,int n2,Graph* G){
  int l;
  double err;


  if(edge_deletion_cost_complex!=NULL){
    
    double err=edge_deletion_cost_complex(A1.Label, A1.complex_value->data, A1.complex_value->descriptor, n1,n2, LabelGroup[l].insertionError,G);
    return err;
  }


  if(edge_deletion_cost_function!=NULL){
     l=lookupLabel(A1.label());
    err=edge_deletion_cost_function(A1.Label,A1.values,A1.n,n1,n2,LabelGroup[l].deletionError,G);
    return err;
  }else{
    
    if(DELETION_COST==0){
      return 0.0;
    }else{
#ifndef NEW_ATTOBJECT
      l=REF[A1.label()].group; 
#else
      l=lookupLabel(A1.label());
#endif
      return LabelGroup[l].deletionError;
    }
  }
}


double
AttributeClass::labelToLabelError(int l1,int l2){

  if(l1==l2) return 0;

  if(CTable[l1][l2].weight==-1) return -1;
    
    if(CTable[l1][l2].weight==-2)
      return CTable[l2][l1].weight;
    else
      return CTable[l1][l2].weight;

}


double 
AttributeClass::error(AttId &A1, AttId &A2){
  int l1,l2;
  int i1,i2;
  double Err,delta;

  Err=0;
  
  error_checks++;

  l1=lookupLabel(A1.label());
  l2=lookupLabel(A2.label());

  
  if(substitution_cost_complex!=NULL){
    Err=substitution_cost_complex(l1,A1.complex_value->data, A1.complex_value->descriptor,
				  l2, A2.complex_value->data, A2.complex_value->descriptor);
    
    return Err;
  }
  
  if(substitution_cost_function_complex!=NULL){
    
    if((A1.complex_value!=NULL)&&(A2.complex_value!=NULL)){
      
      Err=substitution_cost_function_complex(l1,A1.values,A1.n,
					     LabelGroup[l1].weights,LabelGroup[l1].thresholds,
					     l2,A2.values,A2.n,
					     LabelGroup[l2].weights,LabelGroup[l2].thresholds,
					     CTable[LabelGroup[l1].label][LabelGroup[l2].label].weight,
					     CTable[LabelGroup[l2].label][LabelGroup[l1].label].weight,
					     A1.complex_value->data, A1.complex_value->descriptor,
					     A2.complex_value->data, A2.complex_value->descriptor);
    }else{
      Err=substitution_cost_function_complex(l1,A1.values,A1.n,
					     LabelGroup[l1].weights,LabelGroup[l1].thresholds,
					     l2,A2.values,A2.n,
					     LabelGroup[l2].weights,LabelGroup[l2].thresholds,
					     CTable[LabelGroup[l1].label][LabelGroup[l2].label].weight,
					     CTable[LabelGroup[l2].label][LabelGroup[l1].label].weight,
					     NULL,0,
					     NULL,0);
      
    }
    return Err;
    
  }


  if(substitution_cost_function!=NULL){
    Err=substitution_cost_function(l1,A1.values,A1.n,
				   LabelGroup[l1].weights,LabelGroup[l1].thresholds,
				   l2,A2.values,A2.n,
				   LabelGroup[l2].weights,LabelGroup[l2].thresholds,
				   CTable[LabelGroup[l1].label][LabelGroup[l2].label].weight,
				   CTable[LabelGroup[l2].label][LabelGroup[l1].label].weight);
    return Err;
  }else{

 
    if(l1==l2){
      int x;
      
      for(x=0;x<LabelGroup[l1].dim;x++){
	  if(ENV_COST_QUADRATIC)
	      delta=(A1[x]-A2[x])*(A1[x]-A2[x]);
	  else{
	      delta=fabs(A1[x]-A2[x]);

	      if((x==0)&&(LabelGroup[l1].cycleTop)){
		  if(delta>LabelGroup[l1].cycleTop)
		      delta=2*LabelGroup[l1].cycleTop-delta;
	      }
	  }
	  if(delta>LabelGroup[l1].thresholds[x])
	      Err = Err+ delta*LabelGroup[l1].weights[x];
      }
      
      return Err;
      
    }else{
      if(CTable[LabelGroup[l1].label][LabelGroup[l2].label].weight==-1) return -1;
      
      if(CTable[LabelGroup[l1].label][LabelGroup[l2].label].weight==-2)
	return CTable[LabelGroup[l2].label][LabelGroup[l1].label].weight;
      else
	return CTable[LabelGroup[l1].label][LabelGroup[l2].label].weight;
      
    }
  }
}



void
AttributeClass::write(char* name){
  FILE* file;
  int x,y;

  file=fopen(name,"w");
  gmt_assert(file!=NULL);
  
  fwrite((char*)&NumberOfRefs,sizeof(int),1,file);
  
  
  fwrite((char*)REF,sizeof(GroupType),NumberOfRefs+1,file);
  
  fwrite((char*)&NumberOfLabels,sizeof(int),1,file);
  
  for(x=0;x<NumberOfLabels;x++){
    fwrite((char*)&LabelGroup[x].label,sizeof(int),1,file);
    fwrite((char*)&LabelGroup[x].insertionError,sizeof(double),1,file);
    fwrite((char*)&LabelGroup[x].deletionError,sizeof(double),1,file);
    fwrite((char*)&LabelGroup[x].dim,sizeof(int),1,file);
    fwrite((char*)LabelGroup[x].weights,sizeof(double),LabelGroup[x].dim,file);
    fwrite((char*)LabelGroup[x].thresholds,sizeof(double),LabelGroup[x].dim,file);

#ifndef NEW_ATTOBJECT    
    LabelGroup[x].Values.write(file);
#endif 
 }

  for(x=0;x<NumberOfLabels;x++){
    for(y=0;y<NumberOfLabels;y++){

      fwrite((char*)&CTable[x][y],sizeof(double),1,file);
     
/***BUG: missing feature: cannot restore vweights **/
    
    }
  }

  fclose(file);
    
}


int  
AttributeClass::read(char* name){
  FILE* file;
  int x,y;
  
  if(NumberOfLabels>0) discard();
  
  INSERTION_COST=1;
  DELETION_COST=1;

  file=fopen(name,"r");
  
#if 0
  assert(file!=NULL);
#else
  if(!file) return 0;
#endif

  fread((char*)&NumberOfRefs,sizeof(int),1,file);

  fread((char*)REF,sizeof(GroupType),NumberOfRefs+1,file);
  
  fread((char*)&NumberOfLabels,sizeof(int),1,file);
  
  LabelGroup=new AttType[NumberOfLabels];

  for(x=0;x<NumberOfLabels;x++){
    fread((char*)&LabelGroup[x].label,sizeof(int),1,file);
    fread((char*)&LabelGroup[x].insertionError,sizeof(double),1,file);
    fread((char*)&LabelGroup[x].deletionError,sizeof(double),1,file);
    fread((char*)&LabelGroup[x].dim,sizeof(int),1,file);
    LabelGroup[x].weights=new double[LabelGroup[x].dim];
    LabelGroup[x].thresholds=new double[LabelGroup[x].dim];
    fread((char*)LabelGroup[x].weights,sizeof(double),LabelGroup[x].dim,file);
    fread((char*)LabelGroup[x].thresholds,sizeof(double),LabelGroup[x].dim,file);

#ifndef NEW_ATTOBJECT    
    LabelGroup[x].Values.read(file);
#endif 
  }

  /** costTable */

  CTable = new MatrixEntry*[maxLabels];
  for(x=0;x<maxLabels;x++){
    CTable[x]=new MatrixEntry[maxLabels];
    for(y=0;y<maxLabels;y++) {
      CTable[x][y].weight=-2;
      CTable[x][y].vweights=NULL;
    }
  } 

  for(x=0;x<NumberOfLabels;x++){
    for(y=0;y<NumberOfLabels;y++){

      fread((char*)&CTable[x][y],sizeof(double),1,file);
     
/***BUG: missing feature: cannot restore vweights **/
    
    }
  }

  fclose(file);
  return 1;
}


/***************************************/
/*        Hdouble class                */
/***************************************/


#define DEF_SIZE 30


Hdouble::Hdouble(){

     VArray=NULL;
}


Hdouble::~Hdouble(){
  if(VArray)
    delete VArray;
  VArray=NULL;
}



void
Hdouble::discard(){
  int x;

  if(VArray){
    for(x=0;x<size;x++) delete VArray[x];
    delete VArray;
    VArray=NULL;
  }

  size=0;
  dim=0;
  index=0;
}
  

void 
Hdouble::init(int count){
  int x;

  size=DEF_SIZE;
  dim=count;
  index=0;

  VArray=new double*[size];
  
  for(x=0;x<size;x++)
    VArray[x]=new double[count];

}



void
Hdouble::write(FILE* file){
  int x;

  fwrite((char*)&dim,sizeof(int),1,file);
  fwrite((char*)&size,sizeof(int),1,file);
  fwrite((char*)&index,sizeof(int),1,file);

  for(x=0;x<=index;x++)
    fwrite((char*)VArray[x],sizeof(double),dim,file);
    
}


void 
Hdouble::read(FILE* file){
  int x;

  fread((char*)&dim,sizeof(int),1,file);
  fread((char*)&size,sizeof(int),1,file);
  fread((char*)&index,sizeof(int),1,file);

  VArray=new double*[size];
  
  for(x=0;x<size;x++)
    VArray[x]=new double[dim];

  for(x=0;x<=index;x++)
    fread((char*)VArray[x],sizeof(double),dim,file);
    
}



int
Hdouble::add(double* vals){
  int x;

  index++;

  if(index>=size) resize();

  for(x=0;x<dim;x++) VArray[index][x]=vals[x];

  return index;
}  


double
Hdouble::ind(int arr,int i){

  if((arr>size)||(i>dim)){ printf("FATAL: index out of bounds!");gmt_assert(0);};

  return VArray[arr][i];
}


double*
Hdouble::array(int arr){

  if((arr>size)){ printf("FATAL: array out of bounds!");gmt_assert(0);};
  
  return VArray[arr];
}


void
Hdouble::resize(){

  int oldsize,y,x;
  double** Hptr;


  oldsize=size;

  size=2*size;

  Hptr=VArray;

  VArray=new double*[size];
  
  for(x=0;x<oldsize;x++)
    VArray[x]=Hptr[x];

  for(x=oldsize;x<size;x++)
    VArray[x]=new double[dim];

  for(x=oldsize;x<size;x++)
    for(y=0;y<dim;y++) VArray[x][y]=0;

  delete Hptr;

}

