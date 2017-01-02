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

#include "Decision.h"
#include"LineDrawing.h"

int default_valid=1;
int GLOBAL_TAG=0;
int ENV_COMPLETE;
int ENV_MIN_GRAPH_LENGTH=0;
int ENV_BUILD_LEVEL=10;
int ENV_CONSISTENCY_TEST=0;
int ENV_PRUNING_TREE=0;
int ENV_PRUNE_INSTANCE_LIMIT=0;
int ENV_PRUNED_MAX_COUNT=50;
int ENV_COPY=0;
int ENV_DEBUG=0;
int ENV_NOSAVEINSTANCES=0;
int ENV_NOSAVEINSTANCES_ATSAVETIME=0;
int ENV_ACCEPT_FIRST=0;
int ENV_MAX_GRAPH_LENGTH=17;
int ENV_DECISION_TREE_LIMIT=20;
int ENV_FAST_BUILD=0;
int ENV_BYTREE_DIFF=0;
int ENV_ONLY_TRUE_STATES=0;
int ENV_SEARCHING_LINEAR=3;
int ENV_NOREDIRECT=0;
int ENV_NOFASTSEARCH=0;
int return_last_always=0;
int ENV_ERROR_STATIC=0;
int ENV_ERROR_DYNAMIC=0;
double ENV_MAX_ERROR=0;

SEQ_type DUMMY_SEQ;


//Timer Timex("LOG_TIME");


PG_class::PG_class(){
   int x;
   
   subset=NULL;
   subset_dim=0;
   RootNode=NULL;
   numberOfNodes=0;
   numberOfSeq=0;
   Seq=NULL;
   PASSED_BY_NODE=0;
   vertex_checked=NULL;
   AVAIL=NULL;
   saveNamesOfModels=1;

   char* heu;

   /*************** not in this version

   if(heu=getenv("ENV_NOFASTSEARCH")) ENV_NOFASTSEARCH=1;
   if(heu=getenv("ENV_NOREDIRECT")) ENV_NOREDIRECT=1;
   if(heu=getenv("ENV_SEARCHING_LINEAR")) ENV_SEARCHING_LINEAR=atoi(heu);
   if(heu=getenv("ENV_NO_BREADTH_PRUNING")) GLOBAL_TAG=1;
   if(heu=getenv("ENV_COMPLETE")) ENV_COMPLETE=1;
   else ENV_COMPLETE=0;
   if(heu=getenv("ENV_MIN_GRAPH_LENGTH")) ENV_MIN_GRAPH_LENGTH=atoi(heu);
   if(heu=getenv("ENV_BUILD_LEVEL")) ENV_BUILD_LEVEL=atoi(heu);
   if(heu=getenv("ENV_CONSISTENCY_TEST")) ENV_CONSISTENCY_TEST=1;
   if(heu=getenv("ENV_PRUNING_TREE")) ENV_PRUNING_TREE=1;
   if(heu=getenv("ENV_PRUNE_INSTANCE_LIMIT")) ENV_PRUNE_INSTANCE_LIMIT=atoi(heu);
   ENV_PRUNE_INSTANCE_LIMIT=-1;  // bug control

   if(heu=getenv("ENV_COPY")) ENV_COPY=1;
   if(heu=getenv("ENV_DEBUG")) ENV_DEBUG=1;
   if(heu=getenv("ENV_NOSAVEINSTANCES")) ENV_NOSAVEINSTANCES=1;
   if((!ENV_NOSAVEINSTANCES)&&(ENV_SEARCHING_LINEAR<10)){
     printf("WARNING: if ENV_SEARCHING_LINEAR is set, ENV_NOSAVEINSTANCES must be set!\n");
     ENV_NOSAVEINSTANCES=1;
   }

   if(heu=getenv("ENV_NOSAVEINSTANCES_ATSAVETIME")) ENV_NOSAVEINSTANCES_ATSAVETIME=1;

   if(heu=getenv("ENV_ACCEPT_FIRST")) ENV_ACCEPT_FIRST=1;
   if(heu=getenv("ENV_MAX_GRAPH_LENGTH")) ENV_MAX_GRAPH_LENGTH=atoi(heu);
   if(heu=getenv("ENV_DECISION_TREE_LIMIT")) ENV_DECISION_TREE_LIMIT=atoi(heu);
   if(heu=getenv("ENV_PRUNED_MAX_COUNT")) ENV_PRUNED_MAX_COUNT=atoi(heu);
   if(heu=getenv("ENV_FAST_BUILD")) ENV_FAST_BUILD=1;
   if(heu=getenv("ENV_BYTREE_DIFF")) ENV_BYTREE_DIFF=atoi(heu);
   if(heu=getenv("ENV_ONLY_TRUE_STATES")) ENV_ONLY_TRUE_STATES=1;
  
  if(ENV_BUILD_LEVEL<=ENV_MIN_GRAPH_LENGTH)
       ENV_BUILD_LEVEL=ENV_MIN_GRAPH_LENGTH+1;
       
       **************/

   debug=0;
   draw_func=0;
   testing_consistency=0;
  discard();
   AM=NULL;
  BRANCH_class::MODELS_IN_TREE=Models_in_tree;
  setMethod(1,0);
   
}


PG_class::~PG_class(){
 
    discard();
};



int
PG_class::admissibility(Graph* G) {

  if(VARIANTE==1) {
    if(G->numberOfVertices()>19) return 0;
  }else if((VARIANTE==2)||(VARIANTE==4)) {
    if(G->numberOfVertices()>50) return 0;
  }else {
    if(G->numberOfVertices()>22)
      return 0;
  }


  return 1;
}


// Note that the methods for Variant 1-4 are implemented in Decision1.C
// and methods for variant 5-6 are implemented in Decision2.C

void
PG_class::addModel(Graph *G,int num){
  double ts;
  BRANCH_class::MODELS_IN_TREE=Models_in_tree;

  //Timex.start();
  //Timex.start_log();
  //ts=Timex.time_stamp();

  time_spent = elapsed_time();

  if((VARIANTE<5)||(VARIANTE>8)) {
    merge1(G,num);
  }else if(VARIANTE<7) {
    merge2(G,num);
  }else if((VARIANTE==7)||(VARIANTE==8)){
    mergeWithDistortions(G,num);


  }

  //time_spent=Timex.time_stamp()-ts;
  time_spent = elapsed_time() - time_spent;

  //printf("\n Number of nodes in decision tree: %d\n",numberOfNodes); 
}



void
PG_class::setModelDataBase(ModelDataBaseType* MB,int n){
  int x;
  BRANCH_class::MODELS_IN_TREE=Models_in_tree;
  for(x=0;x<n;x++){
    if((x>=MAX_MODELS)||(Models_in_tree[x]!=NULL)){
      cout << "Model number collision! exiting....\n";
      exit(10);
    }

    Models_in_tree[x]=MB[x].G;
    NumberOfModels=x;
    
    if(minSizedModel>MB[x].G->AM.numberOfVertices())
      minSizedModel=MB[x].G->AM.numberOfVertices();
    
    if(maxSizedModel<MB[x].G->AM.numberOfVertices())
      maxSizedModel=MB[x].G->AM.numberOfVertices();
    
  }
}



int
PG_class::recognition(Graph* G) {
  BRANCH_class::MODELS_IN_TREE=Models_in_tree;
  if(RootNode==NULL) return 0;


  if((VARIANTE==1)||(VARIANTE==7)) {

    return isomorph1(G);

  }else if(VARIANTE==2){

    return prunedIsomorphism1(G);

  }else if((VARIANTE==3)||(VARIANTE==8)) {

    return isomorph2(G);

  }else if(VARIANTE==4) {

    return prunedIsomorphism2(G);

  }else if(VARIANTE==5) {
    
    return isomorph2(G);

  }else if(VARIANTE==6) {
    
    return prunedIsomorphism2(G);

  }else if(VARIANTE==9){

    return recognitionWithDistortions(G);

  }

  return 0;
}

void
PG_class::setMethod(int var, double pars) {

  VARIANTE=var;
  PARAMETERS=pars;

  // Default: VARIANTE=1

  ENV_PRUNING_TREE=0;
  ENV_DECISION_TREE_LIMIT=100;
  ENV_BUILD_LEVEL=0;
  ENV_FAST_BUILD=1;
  ENV_BYTREE_DIFF=0;
  ENV_MAX_GRAPH_LENGTH=100;
  GLOBAL_TAG=1;
  ENV_NOSAVEINSTANCES=0;
  ENV_ERROR_STATIC=0;
  ENV_ERROR_DYNAMIC=0;

  switch(VARIANTE) {
  case 1: break;
  case 2: 
    ENV_PRUNING_TREE=1;
    ENV_DECISION_TREE_LIMIT=(int) pars;
    break;
  case 3: 
    GLOBAL_TAG=0;
    ENV_NOSAVEINSTANCES=1;
    break;
  case 4:
    GLOBAL_TAG=0;
    ENV_PRUNING_TREE=1;
    ENV_DECISION_TREE_LIMIT=(int) pars;
    ENV_NOSAVEINSTANCES=1;
    break;
  case 5:
    GLOBAL_TAG=0;
    ENV_NOSAVEINSTANCES=1;
    break;
  case 6:
    GLOBAL_TAG=0;
    ENV_PRUNING_TREE=1;
    ENV_DECISION_TREE_LIMIT=(int) pars;
    break;
  case 7:
    ENV_ERROR_STATIC=1;
    ENV_MAX_ERROR=pars;
    break;
  case 8:
    ENV_ERROR_STATIC=1;
    GLOBAL_TAG=0;
    ENV_NOSAVEINSTANCES=1;
    ENV_MAX_ERROR=pars;
    break;
  case 9:
    ENV_ERROR_DYNAMIC=1;
    ENV_MAX_ERROR=pars;
    break;
  case 10:
    ENV_ERROR_DYNAMIC=1;
    GLOBAL_TAG=0;
    ENV_NOSAVEINSTANCES=1;
    ENV_MAX_ERROR=pars;
    break;

  }

}



int
PG_class::mergeLargeGraph(Graph *G,int num){

  return 0;
}










// auxiliary function
void 
PG_class::tag_op(DT_node* state,int i){

    if(i)
	state->tag++;
    else{
	if(state->tag<=0) return;
	if(GLOBAL_TAG==0)
	    state->tag--;

    }

    if(state->allocated==-1)
	tag_op(state->successors[0],i);
}








DT_node*
PG_class::searchLinearInStates(INT_TYPE* seq,int d){
  int good,x,y;
  INT_TYPE* seq2;
  DT_node* state;

  delInstances2.reset();
  while(state=(DT_node*) delInstances2.getnext()){
    if(state->tag<1) continue;
    if(state->allocated==-1) continue;
    state->sequences.reset();
    while(seq2=(INT_TYPE*) state->sequences.getnext()){
      for(x=0;x<d;x++){
	good=0;
	for(y=0;y<d;y++)
	  if(seq[x]==seq2[y]) good=1;
	if(good==0) break;
      }
      if(good) return state;
    }
  }
  return NULL;
}



int 
PG_class::buildNextLevel(Graph *G,int num,int level){

  return 0;
}



int
PG_class::redirectNode(DT_node* state,int* edges_tuple,INT_TYPE* permutation,int d,DT_node* nstate,INT_TYPE* first_perm, int num,INT_TYPE vertex){
    DT_node* node;
    int x;
   
    node=state;
    if(node->allocated==0) {
      node->successors=new DT_node*[1];
    }

    node->inst_count[0]=0;
    node->models_count=0;
    node->models[0]=num;   
    node->mark=0;
    node->terminal=0;
    node->tag=GLOBAL_TAG;

    node->allocated=-1;
    node->succ=1;
    node->successors[0]=nstate;
    node->permutation=new INT_TYPE[d];
    for(x=0;x<d;x++){
	for(int i=0;i<d;i++)
	    if(permutation[i]==first_perm[x])
		node->permutation[x]=i;
    }

    
    return 0;
}



int
PG_class::addNode_bypass(DT_node* state,int* edges_tuple,INT_TYPE* permutation,int d,DT_node* nstate,INT_TYPE* first_perm, int num,INT_TYPE vertex){
    DT_node* node;
    int x;
    
    if(state->succ==state->allocated){
	DT_node **hpt;

	state->allocated+=MEM_LIMIT;
	hpt=new DT_node* [state->allocated+MEM_LIMIT];
	for(x=0;x<state->succ;x++)
	    hpt[x]=state->successors[x];
	if(state->succ>0)
	    delete[] state->successors;
	state->successors=hpt;
    }

    state->successors[state->succ]=new DT_node;

    node=state->successors[state->succ];
    node->successors=new DT_node*[1];
    node->depth=step+1;
    
    node->models=new int[1];
    node->inst_count= new int[1];
    node->inst_count[0]=0;
    node->models_count=0;
    node->models[0]=num;   
    node->mark=0;
    node->terminal=0;
    node->tag=GLOBAL_TAG;

    node->allocated=-1;
    node->succ=1;
    node->successors[0]=nstate;
    node->permutation=new INT_TYPE[d];
    for(x=0;x<d;x++){
	for(int i=0;i<d;i++)
	    if(permutation[i]==first_perm[x])
		node->permutation[x]=i;
    }

    state->branch.add(edges_tuple,state->succ,step,vertex,AM,num);
    state->succ++;

    numberOfNodes++;
    
    return state->succ-1;
}


int
PG_class::addNode(DT_node* state,int* edges_tuple,int num,INT_TYPE vertex){
    
    
    if(state->succ==state->allocated){
	DT_node **hpt;
	int x;
	state->allocated+=MEM_LIMIT;
	hpt=new DT_node* [state->allocated+MEM_LIMIT];
	for(x=0;x<state->succ;x++)
	    hpt[x]=state->successors[x];
	if(state->succ>0)
	    delete[] state->successors;
	state->successors=hpt;
    }

    state->successors[state->succ]=new DT_node;
//    state->successors[state->succ]->successors=new DT_node* [MEM_LIMIT];
//    state->successors[state->succ]->allocated=MEM_LIMIT;
    state->successors[state->succ]->successors=NULL;
    state->successors[state->succ]->allocated=0;
    state->successors[state->succ]->succ=0;
    state->successors[state->succ]->models=new int[1];
    state->successors[state->succ]->inst_count= new int[1];
    state->successors[state->succ]->inst_count[0]=0;
    state->successors[state->succ]->models_count=0;
    state->successors[state->succ]->models[0]=num;
    state->successors[state->succ]->mark=0;
    state->successors[state->succ]->depth=step+1;
    state->successors[state->succ]->terminal=0;
    state->successors[state->succ]->tag=GLOBAL_TAG;

    state->branch.add(edges_tuple,state->succ,step,vertex,AM,num);
    state->succ++;

    numberOfNodes++;
    
    return state->succ-1;
}





int
PG_class::largeIsomorphism(Graph* G){

  return 0;
}


int
PG_class::largeIso_rec(Graph* G,Hash& USED,List& Partials,Hash &done_segments){

  return 0;
}



int 
PG_class::subisomorph(Graph *G){
    
/*  
 * Span a tree over all possibilities for subsets of vertices in G   
 * if a subgraph is descovered, jump to the root, go back to first level
 * and remove all vertices that are part of the instance from the first level,
 * Next, start with a new vertex, one that has not yet been part of an instance.
 * If no instance was found, remove the top vertex from the list of usable  vertices
 */

  return 0;

   
}



int
PG_class::subisomorph_func(Graph *G,Hash& USED,int starting_vertex){

  return 0;
}


int 
PG_class::dump(char* fname){
   FILE* file;
   BRANCH_class::MODELS_IN_TREE=Models_in_tree;
   char buf[256];
   file=fopen(fname,"w");
  
  sprintf(buf,"Number of Nodes in Decision tree       : %d \n",numberOfNodes);
  fputs(buf,file);
  sprintf(buf,"Number of States in Sequence           : %d \n",numberOfSeq);
  fputs(buf,file);
  sprintf(buf,"Number of Branch Tests                 : %d \n",BRANCH_class::size);
  fputs(buf,file);
  sprintf(buf,"Time spent generating the Decision Tree: %lf \n",time_spent);
  fputs(buf,file);
  
  fclose(file);
  return 1;
}


int 
PG_class::dump(){

  printf("Number of Nodes in Decision tree       : %d \n",numberOfNodes);
  printf("Number of States in Sequence           : %d \n",numberOfSeq);
  printf("Number of Branch Tests                 : %d \n",BRANCH_class::size);
  printf("Time spent generating the Decision Tree: %lf \n",time_spent);
  return 1;
}


int 
PG_class::dumpResults(){

    if(Result_State!=NULL){
	int x;
	printf("Retrieval result:\n");
	for(x=0;x<=Result_State->models_count;x++)
	    printf("Graph: %d Instances: %d\n",Result_State->models[x],Result_State->inst_count[x]);
    }
    printf("\nTime: %lf \n",time_spent);
    return 1;
}


int
PG_class::dumpTree(){
    DT_node* state;
    int x,l;

    state=RootNode;
	
    x=0;
    l=0;
    valid_states=0;
    
    while(recursiveDescent(state,0,l)) {
	l++;    
	cout << "\n";
    }


    cout << "\n Valid States:" << valid_states << "\n";
    return 1;
}


int
PG_class::recursiveDescent(DT_node* state,int level,int limit){
    int s=0;
    int x;

    if(state==NULL) return 0;
 
    if(limit==0){
       cout << "ROOT\n";
       return 1;
     }

    

    if(level==limit){
	if(state->tag>0) valid_states++;
	for(x=0;x<=state->models_count;x++)
	    s+=state->inst_count[x];

	
	cout << s << "(" <<  state->tag << ")";
	if(s==0)
	    cout << "* ";
	else
	    cout << " ";
	
#if 0
	if(level==3){
	    cout << "instances:\n";
	    state->sequences.reset();
	    while(INT_TYPE* seq=(INT_TYPE*) state->sequences.getnext()){
		for(x=0;x<3;x++)
		    cout << seq[x] << ",";
		cout << "\n";
	    }
	}
#endif
	return 1;
    }else{
	int r=0;
	
	

	if(state->allocated==-1) return 0;
	for(x=0;x<state->succ;x++){
	  r+=recursiveDescent(state->successors[x],level+1,limit);
	}
	return r;
    }
}


int 
PG_class::initSequences(int d,int fix){
    int x;    

    vertex_checked_dim=d;
    if(vertex_checked!=NULL)
      delete [] vertex_checked;
    vertex_checked=new int[d];
    vertex_checked_fix=fix;

    if(AVAIL)
      delete [] AVAIL;
    AVAIL=new int[d];

    for(x=0;x<d;x++){
	vertex_checked[x]=0;
	AVAIL[x]=1;
    }
    vertex_checked[0]=-1;
    
    if(vertex_checked_fix)
	vertex_checked[d-1]=-1;

    level=0;
    Seq=NULL;

    return 1;
}


int
PG_class::nextSequences(INT_TYPE* vs){
    SEQ_type* old_Seq,*New_Seq; 
    int x;
    
    // right
    
    if(vertex_checked[level]>-1)
	AVAIL[vertex_checked[level]]=1;
    
    if(vertex_checked_fix){
	/* keep last index fix and try to find
	 *  a valid sequence for the other indices
	 */

	for(x=0;x<vertex_checked_dim;x++)
	    AVAIL[x]=1;
	AVAIL[vertex_checked[vertex_checked_dim-1]]=1;
	vertex_checked[vertex_checked_dim-1]++;
	if(vertex_checked[vertex_checked_dim-1]==vertex_checked_dim)
	    return 0;
	AVAIL[vertex_checked[vertex_checked_dim-1]]=0;
	level=0;
	for(x=0;x<vertex_checked_dim-1;x++)
	    vertex_checked[x]=0;
	vertex_checked[0]=-1;	   
    }

    vertex_checked[level]++;
    while(nonValidSequence())
	vertex_checked[level]++;  
    
    if(vertex_checked[level]==vertex_checked_dim){
	Seq=backup(level,vs,Seq);
    }else{
	if(BUILDING_TREE){
	    old_Seq=Seq;
	    Seq=new SEQ_type;
	    numberOfSeq++;
	    Seq->vertex=vertex_checked[level];
	    if(old_Seq)
		Seq->up=old_Seq->up;
	    else
		Seq->up=NULL;
	}else{
	    Seq=&DUMMY_SEQ;
	}
	AVAIL[vertex_checked[level]]=0;
    }
    
    //forward
    
    if(Seq==NULL) return 0;

    New_Seq=forward(level,vs,Seq);
    

    if(New_Seq==NULL){
	return nextSequences(vs);
    }else{
	
	Seq=New_Seq;

	int i=vertex_checked_dim-1;

	for(i=0;i<vertex_checked_dim;i++)
	    vs[i]=vertex_checked[i];


	return 1; 

    }
}



SEQ_type*
PG_class::backup(int &level,INT_TYPE* vs,SEQ_type* Seq){
    SEQ_type* New_Seq;


    level--;

    if(BUILDING_TREE)
	Seq=Seq->up;

    while((level>=0)&&(vertex_checked[level]==vertex_checked_dim-1)){
	
	if(BUILDING_TREE) 
	    Seq=Seq->up;
	AVAIL[vertex_checked[level]]=1;
	level--;
	
    }
    if(level<0) return NULL;
    
    AVAIL[vertex_checked[level]]=1;
    vertex_checked[level]++;
    while(nonValidSequence()){
	vertex_checked[level]++;
    }
    
    if(vertex_checked[level]==vertex_checked_dim){
	return backup(level,vs,Seq);
    }else{
	AVAIL[vertex_checked[level]]=0;
	if(BUILDING_TREE){
	    New_Seq=new SEQ_type;
	    New_Seq->up=Seq->up;
	    numberOfSeq++;
	    New_Seq->vertex=vertex_checked[level];
   
	    return New_Seq;
	}else{
	    return &DUMMY_SEQ;
	}
	
    }
}


SEQ_type*
PG_class::forward(int &level,INT_TYPE* vs,SEQ_type* Seq){
    SEQ_type* New_Seq;

    int old_level=level;
    level++;
    int rec=0;
    while(level<vertex_checked_dim-vertex_checked_fix){

	vertex_checked[level]=rec;
	while(nonValidSequence()){
	    vertex_checked[level]++;
	}

	if(vertex_checked[level]==vertex_checked_dim){
	    level--;
	    if(level==old_level) 
		return NULL;
	    AVAIL[vertex_checked[level]]=1;
	    rec=vertex_checked[level]+1;
	}else{
	    rec=0;
	    AVAIL[vertex_checked[level]]=0;
	 
	    if(BUILDING_TREE){
		New_Seq=new SEQ_type;
		New_Seq->vertex=vertex_checked[level];
		New_Seq->up=Seq;
		numberOfSeq++;
		Seq=New_Seq;
	    }else{
		Seq=&DUMMY_SEQ;
	    }
	    level++;
	}
    }

    level--;
    return Seq;
}


int
PG_class::nonValidSequence(){
    int i;

    

    if(vertex_checked_fix==0)
	if(level==0) return 0;
    if(vertex_checked[level]<vertex_checked_dim){
	if((AVAIL[vertex_checked[level]]==0))
	    return 1;
	else{
	    if(AM==NULL) return 0;
	    if(default_valid) return 0;
	    i=level-1;
	    while((i>=0)&&(AM->isEdge(subset[vertex_checked[level]],subset[vertex_checked[i]])==-1))
		i--;
	    
	    if(vertex_checked_fix){
		if((i<0)&&(level==0))
		    return 0;
	    }
	    
	    if(i<0)
		return 1;  // BUG: return 1;
	    else
		return 0;
	}
    }else
	return 0;
    
}



void
PG_class::initSubset(int d,int max){
    
    // the vertices in AM are numbered from 0 to n-1
    // therefore setindex indicates the current subset
    // by simple numbers!

    if(subset!=NULL){
	delete[] subset;
	subset=NULL;
    }

    subset=new int[d];
    
    for(int x=0;x<d;x++)
	subset[x]=x;

    subset[d-1]--;
    subset_dim=d;
    subset_lim=max;
}


int
PG_class::nextSubset(){

    int x,y;
    
    x=subset_dim-1;

    while(x>=0){
	subset[x]++;


	if(subset[x]+(subset_dim-x-1)>=subset_lim){
 	    x--;
 	    continue;
	}

	if(subset[x]<subset_lim){
	    int o=subset[x]+1;
	    for(y=x+1;(y<subset_dim)&&(o<subset_lim);y++){
		subset[y]=o++;
	    }
	    if((y==subset_dim)&&(o<=subset_lim))
		return 1;
	}
	x--;
    }
    return 0;
		
}


DT_node*
PG_class::makeOneStep(DT_node* state,int d){
    int x;
    
    DT_node* hstate=state;
    
/* 
   follow any link that connects a permutation node 
   to a real state node, by rearranging the
   necessary nodes
*/

    while(hstate->allocated==-1){
	checks++;
	PASSED_BY_NODE=1;
	for(x=0;x<d;x++) H_seq[x]=vertex_sequence[x+current_start_pos];
	for(x=0;x<d;x++)
	    vertex_sequence[x+current_start_pos]=H_seq[hstate->permutation[x]];

	if((draw_func)&&(!ENV_ONLY_TRUE_STATES))
		draw_func(hstate->mark,hstate->successors[0]->mark);
	hstate=hstate->successors[0];
    }

    return hstate;

}



DT_node*
PG_class::makeOneStepInMerge(DT_node* state,int d,INT_TYPE* permutation){
    int x;
    
    DT_node* hstate=state;
    
/* 
   follow any link that connects a permutation node 
   to a real state node, by rearranging the
   necessary nodes
*/

    while(hstate->allocated==-1){
	checks++;
	PASSED_BY_NODE=1;
	for(x=0;x<d;x++) H_seq[x]=vertex_sequence[x];
	for(x=0;x<d;x++)
	    vertex_sequence[x]=H_seq[hstate->permutation[x]];

	for(x=0;x<d;x++) H_seq[x]=permutation[x];
	for(x=0;x<d;x++)
	    permutation[x]=H_seq[hstate->permutation[x]];

	hstate=hstate->successors[0];
    }

    return hstate;

}


int 
PG_class::consistencyTest(Graph* G){
    DT_node* state;
    AM=&G->AM;
    int i,x,next;
    INT_TYPE* permutation;
    BRANCH_class::MODELS_IN_TREE=Models_in_tree;

    testing_consistency=1;

    if(RootNode==NULL){
	RootNode=new DT_node;
	RootNode->successors=new DT_node* [MEM_LIMIT];
	RootNode->allocated=MEM_LIMIT;
	RootNode->succ=0;
    }
    state=RootNode;
    step=0;
    Dim=AM->Dimension;
    permutation=new INT_TYPE[Dim];
    vertex_sequence=new INT_TYPE[Dim];
    edges_tuple=new int[Dim];
    
    H_seq=new int[Dim];
    
    int first_permutation;
    DT_node* first_state;
    
    initSubset(Dim,Dim);

    while(nextSubset()){
	initSequences(Dim,0); // on subset
    
	while(nextSequences(permutation)){
	    i++;
	    for(x=0;x<Dim;x++)
		vertex_sequence[x]=subset[permutation[x]];
	
	
	    state=RootNode;
	    step=0;


	    if(ENV_PRUNING_TREE){
	      if(VARIANTE>2) {
		if(!prunedIsomorphism2(G)) state=NULL;
	      }else {
		if(!prunedIsomorphism1(G)) state=NULL;
	      }
	    }else{
	      if(VARIANTE>2)
		state=run2(RootNode,0,Dim);
	      else
		state=run1(RootNode,0,Dim);
	    }
	    if(state==NULL){
		cout << "Original Sequence: ";
		for(x=0;x<Dim;x++)
		    cout << permutation[x] << " ";
		    
		cout << "\nFailed Sequence  : ";
		for(x=0;x<Dim;x++)
		    cout << vertex_sequence[x] << " ";
 
		cout << "\n ERROR IN CONSISTENCY TEST!\n";

		debug=1;
		for(x=0;x<Dim;x++)
		    vertex_sequence[x]=subset[permutation[x]];
		
		step=0;
		state=run2(RootNode,0,Dim);
		
		debug=0;

		gmt_assert(0);
	    }


	    first_permutation=0;
	    
	}
	delete [] vertex_checked;
	delete [] AVAIL;
	AVAIL=NULL;
    }
    delete [] H_seq;
    delete [] vertex_sequence;
    delete [] edges_tuple;

    testing_consistency=0;
    
    return 1;
    
  }



void
PG_class::setNodeData(DT_node* state,int num){

    int &hl=state->models_count;
    int *H;
    int* I;

    
    if(state->models[hl]!=num){
	H=state->models;
	I=state->inst_count;
	hl++;
	state->models=new int[hl+1];
	state->inst_count=new int[hl+1];
	for(int x=0;x<hl;x++){
	    state->models[x]=H[x];
	    state->inst_count[x]=I[x];
	}
	state->models[hl]=num;
	state->inst_count[hl]=0;
	delete [] H;
	delete [] I;
    }
  
}


void
PG_class::addInstance(DT_node* state, INT_TYPE* sequence){
   int x;
   INT_TYPE *c_seq;
   Indicator_type* indicator;

   state->inst_count[state->models_count]++;
  
   if(state->depth>=ENV_MIN_GRAPH_LENGTH){
       
	   c_seq=new INT_TYPE[state->depth];
	   for(x=0;x<state->depth;x++)
	       c_seq[x]=sequence[x];
	   state->sequences.insert(c_seq);
	   indicator=new Indicator_type;
	   indicator->node=NULL;
	   state->sequences_indicator.insert(indicator);

   }
} 

       
int
PG_class::recursiveCollectStates(List& STATES,DT_node* state,int l,int level){

    int x;
    
    if(state==NULL) return 0;
    if(state->allocated==-1) return 0;

    if(l==level){

      
      STATES.insert(state);
	
	return 1;
    }else{
	int r=0;

	
	for(x=0;x<state->succ;x++){
	  r+=recursiveCollectStates(STATES,state->successors[x],l+1,level);
	}
	return r;
    }
}


void
PG_class::collectMatches(INT_TYPE* vertex_sequence,DT_node* state){
   int x,xx,y,i,t;
   Token* tok1,*tok2;
   MappingData *MP;
   INT_TYPE* seq;
   int orig;
   int *v;

   i=0;
   for(x=0;x<=state->models_count;x++){
     
 //      if(Models_in_tree[state->models[x]]->numberOfVertices()!=state->depth) continue;
       for(xx=0;xx<state->inst_count[x];xx++){
      
	   seq=(INT_TYPE*) state->sequences.get(i);
	   MP= new MappingData;
	
	   
	   if(OriginalModels.in(state->models[x])){
	     v=(int*) OriginalModels.getKey(state->models[x]);
	     orig=*v;
	   }else{
	     orig=state->models[x];
	   }
	   
	   MP->setNumber(orig);
	   
	   tok1=new Token(state->depth);
	   tok2=new Token(state->depth);
	   
	   for(y=0;y<state->depth;y++){
	       tok1->set(y,seq[y]);
	       tok2->set(y,vertex_sequence[y]);
	   }
	   
	   if(VARIANTE>8)
	     calculateError(tok1,tok2,&Models_in_tree[orig]->AM,OriginalAM);
	   else
	     calculateError(tok1,tok2,&Models_in_tree[orig]->AM,AM);


	   MP->setMatch(tok2,tok1);
	   Collection.insert(MP,orig,tok2->totalErr());
	   delete tok1;
	   delete tok2;
	   i++;
       }
   }
}



void
PG_class::calculateError(Token* tok1,Token* tok2,AdjazenzMatrix *AM1,AdjazenzMatrix *AM2){
  int x,y;
  //  int l,r;
  AttId oat,at;
  double sumErr,err,d;
  int *path;
  AttId e_l[4],e_r[4];
  int edge_l[4],edge_r[4];
  int edg,g;

  // NOT MORE THAN 4 MULTIPLE EDGES !!
/* Vertex Label error caculation  */

  sumErr=0;
  for(x=0;x<tok1->length();x++){
    oat=AM1->getNodeAttributeId((*tok1)[x]);

    at=AM2->getNodeAttributeId((*tok2)[x]);    
    err=ATT_object.error(oat,at);
    tok2->setErr(x,err);
    
 
  }

  
/* Edge Label error calculation  */

  int directed;
  d=0;
  for(x=0;x<tok1->length();x++){
    for(y=x;y<tok1->length();y++){ 

      double **m;
     
      int dim_l,dim_r;
      int i,j,z;

      //      l=C->Gene(x);
      //      r=C->Gene(y);
      
      if(AM1->isDirected()) directed=2; else directed=1;
      for(z=0;z<directed;z++){
	
	if((z==1)&&(x==y)) continue;
	if(z==0)
	  AM1->initNext((*tok1)[x],(*tok1)[y]);
	else
	  AM1->initNext((*tok1)[y],(*tok1)[x]);
       
	dim_l=0;dim_r=0;
	
	while((oat=AM1->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	  e_l[dim_l]=oat;
	  edge_l[dim_l++]=edg;
	  
	}

	edge_l[dim_l]=-1;
	e_l[dim_l++].set(-1,NULL,0);
	

	if(((*tok2)[x]<0)||((*tok2)[y]<0)){
	  for(i=0;i<dim_l-1;i++){
	    Edit_Desc *eds;
	    eds=new Edit_Desc;
	    eds->edge_o=edge_l[i];
	    eds->err=ATT_object.deletionCostOfEdge(e_l[i],(*tok2)[x],(*tok2)[y],(Graph*) AM2->owner);
	 
	   
	  }
	}else{

	  if(z==0)
	    AM2->initNext((*tok2)[x],(*tok2)[y]);
	  else
	    AM2->initNext((*tok2)[y],(*tok2)[x]);
	  
	  while((at=AM2->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	    e_r[dim_r]=at;
	    edge_r[dim_r++]=edg;
	  }

	  edge_r[dim_r]=-1;
	  e_r[dim_r++].set(-1,NULL,0);

	  if(dim_l*dim_r>1){
	    
	    m=new double*[dim_l];
	    for(i=0;i<dim_l;i++) m[i]=new double[dim_r];
	    
	    for(i=0;i<dim_l;i++)
	      for(j=0;j<dim_r;j++){
		if(i==dim_l-1){
		  if(j<dim_r-1)
		    m[i][j]=ATT_object.insertionCostOfEdge(e_r[j],1,1,(Graph*) AM1->owner);
		}else{
		  if(j<dim_r-1)
		    m[i][j]=ATT_object.error(e_l[i],e_r[j]);
		  else{
		    if(z==0)
		      m[i][j]=ATT_object.deletionCostOfEdge(e_l[i],(*tok2)[x],(*tok2)[y],(Graph*) AM2->owner);
		    else
		      m[i][j]=ATT_object.deletionCostOfEdge(e_l[i],(*tok2)[y],(*tok2)[x],(Graph*) AM2->owner);
		  }
		} 
	      }
	    
	    path=new int[dim_l];

	   
	    d+=HS_smart(m,dim_l,dim_r,path);
	    	   

	    //collect_edge_edits(dim_l,dim_r,path,edge_l,edge_r,m,SUB_list,DEL_list,INS_list);

	    for(i=0;i<dim_l;i++) delete m[i];
	    delete m;
	    delete path;
	  }
	}   
	if(!AM2->isDirected()) break;
      }
    }
  }

  tok2->setEdgeErr(d);
  

}




void
PG_class::collectPartialMatches(INT_TYPE* vertex_sequence,DT_node* state){
   int x,xx,y,i,t;
   
   Type_part* spart;
   Type_largePart *part;
   int* seq;

   i=0;
   x=0;
#if 0  
 for(x=0;x<=state->models_count;x++){
     
      
       for(xx=0;xx<state->inst_count[x];xx++){
#endif
       if(state->models_count>=0){ 
	   spart=(Type_part*) state->sequences.get(i);
	   
	   part=new Type_largePart;
	   part->seg=spart->seg;
	   part->original=spart->seq;
	   part->length=state->depth;
	   part->state=state;
	   part->model=x;
	   part->sequence=new int[state->depth];
	   memcpy(part->sequence,vertex_sequence,state->depth*sizeof(int));
	   
	   PartialMatches.insert(part);
	   
	   i++;
       }
 //  }
}


int
PG_class::NumberOfMatches(){
   

   return Collection.count();

}


MappingData*
PG_class::query(int c){
  int x;

  return (MappingData*) Collection.get(c);
  

  
}


void
PG_class::clear(){ 
   MappingData* mp;
   int key;
   BRANCH_class::MODELS_IN_TREE=Models_in_tree;

   while(Collection.count()>0){
     mp=(MappingData*) Collection.removeTop(&key);
     delete mp;
   }

   
   models_checked=0;
   Result_State=NULL;

   //used when the tree is pruned
   current_start_pos=0;
}


void
PG_class::discard(){
    BRANCH_class::MODELS_IN_TREE=Models_in_tree;

    minSizedModel=1000;
    maxSizedModel=0;
    for(int x=0;x<MAX_MODELS;x++)
	Models_in_tree[x]=NULL;


    deleteRecursively(RootNode);
    RootNode=NULL;
    if(subset!=NULL)
	delete [] subset;
    subset=NULL;
    if(AVAIL)
      delete [] AVAIL;
    AVAIL=NULL;
    if(vertex_checked)
      delete [] vertex_checked;
    vertex_checked=NULL;

    clear();
    OriginalModels.clear();

}


void
PG_class::deleteRecursively(DT_node* state){

    if(state==NULL) return;

    if(state->allocated>-1){
	for(int x=0;x<state->succ;x++)
	    deleteRecursively(state->successors[x]);
    }

    while(state->sequences_indicator.count())
	delete (Indicator_type*) state->sequences_indicator.remove(0);

    while(state->sequences.count())
	delete [] (INT_TYPE*) state->sequences.remove(0);

    if(state->allocated==-1)
	delete[] state->permutation;
    delete [] state->models;
    delete [] state->inst_count;
    delete [] state->successors;
    delete state;

}



int
PG_class::write(char* name){
    FILE* file;
    int x=0;

    statesWritten=0;
    BRANCH_class::MODELS_IN_TREE=Models_in_tree;

    confirmValidStates();
    enumerateStates(RootNode,x);

    file=fopen(name,"w");
    
    writeState(RootNode,file);

    fclose(file);



    if(saveNamesOfModels){
      char buf[1024];
      
      strcpy(buf,name);
      strcat(buf,".glist");
      file=fopen(buf,"w");
      for(x=0;x<MAX_MODELS;x++){
	if(Models_in_tree[x]==NULL)
	  fputs("---\n",file);
	else{
	  fputs(Models_in_tree[x]->Name(),file);
	  fputs("\n",file);
	}
      }
      fclose(file);
    }

    cout << "States written to file: " << statesWritten << "\n";
    return 1;
}


void
PG_class::positionStates(Hash& STATES, int*& Rows,int& Depth){
    int x;
    Rows=new int[maxSizedModel+1];
    Depth=maxSizedModel+1;

    if(ENV_PRUNING_TREE)
	if(Depth>ENV_DECISION_TREE_LIMIT) Depth=ENV_DECISION_TREE_LIMIT+1;
   
    for(x=0;x<Depth;x++) Rows[x]=0;

    
    x=0;
    enumerateAndLocateStates(RootNode,x,Rows,STATES);

    

}


void 
PG_class::drawConnections(void (*func(int,int)), int all){

    
    enumerateAndDrawStates(RootNode,func,all);
    
}


void
PG_class::enumerateAndDrawStates(DT_node* state, void (*draw(int,int)),int all){

    if(state==NULL) return;

    if((all==0)&&(state->tag<=0)) return;
  
    if(state->allocated>-1){
	for(int x=0;x<state->succ;x++){
	    if(state->successors[x]==NULL) continue;
	    
	    if((all)||(state->successors[x]->tag>0)){

		if(((state->depth+1)==state->successors[x]->depth)&&
		    (state->successors[x]->tag>=0)&&
		    (state->tag>=0)){
		    if((!ENV_ONLY_TRUE_STATES)||
		       ((state->successors[x]->allocated>-1)&&
			(state->allocated>-1))){
			draw(state->mark,state->successors[x]->mark);
		    }else{
			DT_node* hstate=state->successors[x];
			while(hstate->allocated==-1) hstate=hstate->successors[0];
			draw(state->mark,hstate->mark);
		    }
		}


		enumerateAndDrawStates(state->successors[x],draw,all);
	    }
	}
    }
}



void
PG_class::enumerateAndLocateStates(DT_node* state,int &m,int* Rows,Hash &STATES){
    RC_type* rc;

    if(state==NULL) return;

    state->mark=m;

    if(state->tag>=0){
	if((!ENV_ONLY_TRUE_STATES)||(state->allocated>-1)){
	    rc=new RC_type;
	    rc->row=state->depth;
	    rc->column=Rows[state->depth];
	    STATES.insert(rc,m);
	    
	    Rows[state->depth]++;
	}    
    }

    if(state->allocated>-1){
	
	int x;
	for(x=0;x<state->succ;x++){
	    if(state->successors[x]==NULL) continue; 
	    if(state->successors[x]->allocated!=-1){
		m++;
		enumerateAndLocateStates(state->successors[x],m,Rows,STATES);
	    }
	}
	for(x=0;x<state->succ;x++){
	    if(state->successors[x]==NULL) continue; 
	    if(state->successors[x]->allocated==-1){
		m++;
		enumerateAndLocateStates(state->successors[x],m,Rows,STATES);
	    }
	}
	
    }
}


void
PG_class::enumerateStates(DT_node* state,int &m){

    if(state==NULL) return;

    if(state->mark!=30000) state->tag=0;
    state->mark=m;
    
    if(state->allocated>-1){
	for(int x=0;x<state->succ;x++){
	    m++;
	    enumerateStates(state->successors[x],m);
	}
    }
}


int 
PG_class::writeState(DT_node* state,FILE* file){
    int flag=0;
    int sf=sizeof(int);
    int SC=sizeof(INT_TYPE);
    int tag=1;

    if((state==NULL)){ 
	fwrite((char*) &flag,sf,1,file);
	return 0;
    }

    int n_allocated,n_succ;
    DT_node** n_successors=NULL;

    if(state->tag<=0){
	
	tag=0;
	// create an intermediate state, which stores and reads all
	// necessary states below the current one, but is not
	// referenced by any other state.

	List States;
	
	if(state->allocated>-1){
	    DT_node* tstate;

	    searchNecessaryStates(States,state);
	    
	    n_allocated=States.count();
	    n_succ=n_allocated;
	    n_successors=new DT_node*[n_allocated];
	    
	    int i=0;
	    States.reset();
	    while(tstate=(DT_node*) States.getnext()){
		n_successors[i++]=tstate;
	    }
	    
	}else{
	    fwrite((char*) &flag,sf,1,file);
	    return 0;
	}
	
    }

    flag=1;
    fwrite((char*) &flag,sf,1,file);


    statesWritten++;

    state->branch.write(file);

    if(state->tag>0)
	fwrite((char*) &state->allocated,sf,1,file);
    else
	fwrite((char*) &n_allocated,sf,1,file);
    
    
    fwrite((char*) &state->models_count,sf,1,file);
    fwrite((char*) &state->terminal,SC,1,file);
    fwrite((char*) &state->mark,sf,1,file);
    fwrite((char*) &state->depth,SC,1,file);
    fwrite((char*) &tag,sf,1,file);


    fwrite((char*) state->models, sf,state->models_count+1,file);
    fwrite((char*) state->inst_count, sf, state->models_count+1,file);
    
    if(state->allocated==-1){
	fwrite((char*) state->permutation, SC, state->depth,file);
    }else{
	INT_TYPE* sq;
	int c=state->sequences.count();
	if((state->terminal==0)&&(state->succ>0)&&(ENV_NOSAVEINSTANCES_ATSAVETIME)){
	    c=0;
	}

	fwrite((char*) &c, sf, 1,file);
	if(c>0){
	    state->sequences.reset();
	    while(sq=(INT_TYPE*) state->sequences.getnext())
		fwrite((char*) sq,SC,state->depth,file);
	}
    }


    if(state->tag>0)
	fwrite((char*) &state->succ,sf,1,file);
    else
	fwrite((char*) &n_succ,sf,1,file);

    if(state->allocated>-1){
	if(state->tag>0){
	    for(int x=0;x<state->succ;x++)
		writeState(state->successors[x],file);
	}else{
	     for(int x=0;x<n_succ;x++)
		writeState(n_successors[x],file);
	}    
    }

    if(state->allocated==-1){
	flag=state->successors[0]->mark;
	fwrite((char*) &flag,sf,1,file);
    }

    if(n_successors!=NULL) delete [] n_successors;
    return 1;

}



int
PG_class::read(char* name){
    FILE* file;
    BRANCH_class::MODELS_IN_TREE=Models_in_tree;

    file=fopen(name,"r");
    if(!file) return 0;

    discard();
    NumberOfModels=1;

    RootNode=readState(file);

    fclose(file);

    // restore permutation pointers

    DT_node* state,*to_state;
    int mark,key;

    while(Leftside.count()){
	mark=(int) Leftside.TopValue();
	state=(DT_node*) Leftside.removeTop(&key);
	to_state=(DT_node*) REF.getKey(mark);
	state->successors=new DT_node*[1];
	state->successors[0]=to_state;
    }

    REF.clear2();

    
    char buf[1024];
    char text[1024];
    Graph *G,*G2;



    if(saveNamesOfModels){
      strcpy(buf,name);
      strcat(buf,".glist");
      file=fopen(buf,"r");
      int num=-1;
      while(fgets(text,256,file)){
	num++;
	gmt_assert(num<MAX_MODELS);	
	if(strstr(text,"---")){
	  Models_in_tree[num]=NULL;
	}else{
	  char* nn=strtok(text,"\n");
	  if(strstr(nn,".eepic")){
	    
	    G2=extract_line_graph_from_eepic(nn,5,5+1,0.3);
	    G=build_complex_vertices(G2, 5-1,0.3);
	    
	  }else if(strstr(nn,".grp")){
	    G=new Graph;
	    G->read(nn);
	    
	  }else{
	    
	    G=new Graph;
	    G->AM.readFromFile(&ATT_object,nn);
	    G->defaultKeys();
	  }
	  if(!G)
	    printf("Graph %s could not be reloaded!\n",nn);
	  else{
	    G->setName(nn);
	    
	    Models_in_tree[num]=G;
	    if(G->numberOfVertices()>maxSizedModel)
	      maxSizedModel=G->numberOfVertices();
	    
	    if(G->numberOfVertices()<minSizedModel)
	      minSizedModel=G->numberOfVertices();
	    
	    NumberOfModels++;
	  }
	}
      }
    }
    BRANCH_class::MODELS_IN_TREE=Models_in_tree;
    fclose(file);

   
}


DT_node* 
PG_class::readState(FILE* file){
    DT_node* state;
    int flag=0;
    int tag=1;
    int sf=sizeof(int);
    int SC=sizeof(INT_TYPE);
    
    
    fread((char*) &flag,sf,1,file);
    
    if(!flag) return NULL;

    state=new DT_node;
    state->successors=NULL;

    state->branch.read(file);

    fread((char*) &state->allocated,sf,1,file);
    fread((char*) &state->models_count,sf,1,file);
    fread((char*) &state->terminal,SC,1,file);
    fread((char*) &state->mark,sf,1,file);
    fread((char*) &state->depth,SC,1,file);
    fread((char*) &tag,sf,1,file);


    if(state->terminal){
	if(state->depth<minSizedModel)
	    minSizedModel=state->depth;
    }

    state->models=new int[state->models_count+1];
    fread((char*) state->models, sf,state->models_count+1,file);
    state->inst_count=new int[state->models_count+1];
    fread((char*) state->inst_count, sf, state->models_count+1,file);
    
    if(state->allocated==-1){
	state->permutation=new INT_TYPE[state->depth];
	fread((char*) state->permutation, SC, state->depth,file);
    }else{
	INT_TYPE* sq;
	int c;
	fread((char*) &c, sf, 1,file);
	if(c>0){
	    for(int x=0;x<c;x++){
		sq=new INT_TYPE[state->depth];
		fread((char*) sq,SC,state->depth,file);
		state->sequences.insert(sq);
	    }
	}
    }

    fread((char*) &state->succ,sf,1,file);

    if(state->allocated>-1){
	state->allocated=state->succ;
	state->successors=new DT_node*[state->succ];
	for(int x=0;x<state->succ;x++)
	    state->successors[x]=readState(file);
	
    }

    if(state->allocated==-1){	
	fread((char*) &flag,sf,1,file);
	
	// insert into a list which contains ref pointers under a special mark

	Leftside.insert(state,flag,flag);

    }

    REF.insert(state,state->mark);

    if(tag==0)
	state->tag=-1;
    else
	state->tag=tag;

    if(tag>0)
	return state;
    else 
	return state;

}



int
PG_class::numberOfModels(){

    return NumberOfModels;

}


Graph*
PG_class::getModel(int i){

    while((Models_in_tree[i]==NULL)&&(i<MAX_MODELS)) i++;
    return Models_in_tree[i];

}

    
int 
PG_class::collectPermutations(DT_node* state, int level, List& Perms, List& IND, int num){
    INT_TYPE* seq;
    Hash ht(100);
    int diff,x,z;
    Indicator_type* indicator;
    
    z=0;


    int s=0;
    int model_is_in=0;
    for(x=0;x<=state->models_count;x++)
	if(state->models[x]!=num)
	    s+=state->inst_count[x];
	else{
	    model_is_in=1;
	    break;
	}


    state->sequences.reset();
    state->sequences_indicator.reset();

    int j=-1;

    if(state->sequences.count()==0) return 1;

    if(model_is_in){
	for(x=0;x<level;x++)
	    ht.insert(vertex_sequence[x]);
	
    }else{

	gmt_assert(0);
	
	// take last model as reference, should never happen
	s=0;
	for(x=0;x<state->models_count;x++)
	    s+=state->inst_count[x];
	seq=(INT_TYPE*) state->sequences.getnext();
	indicator=(Indicator_type*) state->sequences_indicator.getnext();
	for(x=0;x<level;x++)
	    ht.insert(seq[x]);
	Perms.insert(seq);
	IND.insert(indicator);
	j=s-1;
    }

    


    while(seq=(INT_TYPE*) state->sequences.getnext()){
	indicator=(Indicator_type*) state->sequences_indicator.getnext();

	diff=0;
	j++;
	if(j<s) continue;


	for(x=0;x<level;x++)
	    if(!ht.in(seq[x])) {
		diff=1;
		break;
	    }
	if(!diff){
	    Perms.insert(seq);
	    IND.insert(indicator);
	}
    }


    if(model_is_in==0){
	// if model is not yet in this node, then we must adjust the found
	// permutations
	Hash ref(10);
	int *r;

	seq=(INT_TYPE*) Perms.get(0);
	for(x=0;x<level;x++){
	    r=new int;
	    *r=x;
	    ref.insert(r,seq[x]);
	}

	List hperms;
	INT_TYPE *hseq;
	Perms.mv(hperms);

	while(hperms.count()){
	    seq=(INT_TYPE*) hperms.remove(0);
	    hseq=new INT_TYPE[level];
	    for(x=0;x<level;x++){
		r=(int*) ref.get(seq[x]);
		hseq[x]=vertex_sequence[*r];
	    }
	    Perms.insert(hseq);
	}
    }

    return model_is_in;

}



int
PG_class::orderInLine(INT_TYPE* vs,int start,int level,int coherent_length){

    int x,y;
    int good,switch_with,h;

    if(start==0) return 1;

    if(Dim<coherent_length)
	coherent_length=Dim;

    for(x=start;x<coherent_length;x++){
	good=0;
	switch_with=x+1;
	while((!good)){
	    for(y=current_start_pos;y<x;y++){
		if(AM->isEdge(vs[x],vs[y])>-1){
		    good=1;
		    break;
		}
	    }
	    if(!good){
		if(switch_with>=level){
		    break;
		}
		h=vs[switch_with];
		vs[switch_with]=vs[x];
		vs[x]=h;
		switch_with++;
   
	    }
	}
	if(!good) return 0;
    }
    return 1;
}


void 
PG_class::getStatistics(double& time,int& size,int& checksi){

    time=time_spent;
    size=numberOfNodes;
    checksi=checks;

}


void 
PG_class::getStatistics(int *expansion,int* memory,long* time,int* instances,double* minError, int* checksi) {

  *time=(long) time_spent;
  *memory=numberOfNodes;
  *checksi=checks;
  *instances=NumberOfMatches();
  *minError=Collection.TopValue();


}


int
PG_class::recSegments(INT_TYPE** segments,int* AVAIL,int* seg_length,int cur){
    Hash USED, already_adj;
    int x,y,z;
    int length;
    int* v;
    int lstep;
    
       // init

    z=AM->numberOfVertices()-((cur)*ENV_MAX_GRAPH_LENGTH);
    if(z<ENV_MAX_GRAPH_LENGTH)
	length=z;
    else
	length=ENV_MAX_GRAPH_LENGTH;

    if(length<=0)
	return 1;

    seg_length[cur]=length;
    List* adj_list;
    adj_list= new List[length];

    if(cur==0){
	
	v=new int;
	*v=0;
	adj_list[0].insert(v);
    
    }else{
	
	for(x=0;x<AM->numberOfVertices();x++){
	    if(AVAIL[x]){
	    v=new int;
	    *v=x;
	    adj_list[0].insert(v);
	    }
	}
    }

    lstep=0;
    adj_list[0].reset();

    while((lstep>=0)&&(lstep<length)){

	
	v=(int*) adj_list[lstep].getnext();
	

	if(v==NULL){// retract to previous level, restore vertex_sequence!

	    adj_list[lstep].reset();
	    while(adj_list[lstep].count()){
		v=(int*) adj_list[lstep].remove(0);
		USED.remove(*v);
		AVAIL[*v]=1;
		delete v;
	    }
	    
	    lstep--;

	    continue;
	}


	USED.insert(*v);
	segment[cur][lstep]=*v;

	lstep++;

	int out,in;
	AttId at;

	if(lstep<(length)){
	    // expand this state
	    // collect all adjacent vertices that are not in use
	    already_adj.clear2();
	    
	    for(x=0;x<lstep;x++){
		AM->initNext(segment[cur][x]);
		while(AM->getnext(&out,&in,&at)!=-1){
		    if(out==segment[cur][x])
			y=in;
		    else
			y=out;
		    if((!USED.in(y))&&(!already_adj.in(y))){
			if(AVAIL[y]){
			    v=new int;
			    *v=y;
			    adj_list[lstep].insert(v);
			    already_adj.insert(NULL,*v);
			}
		    }
		}
	    }
	
	    adj_list[lstep].reset();
	

	}else if(lstep==(length)){

	    for(x=0;x<length;x++)
		AVAIL[segment[cur][x]]=0;

	    if(recSegments(segment,AVAIL,seg_length,cur+1)){

		already_adj.clear2();
		delete [] adj_list;

		return 1;
	    }

	    lstep--;
	    for(x=0;x<length;x++)
		AVAIL[segment[cur][x]]=1;
	}
    }

    delete [] adj_list;
    return 0;
}


void
PG_class::collectPrunedInstances(int* start_pos,int* end_pos,DT_node** the_state,int parts,INT_TYPE* vertex_sequence){
    int x,y,z,i,j;
    int d,good;
    INT_TYPE** seq;
    seq=new INT_TYPE*[parts];
    int en1,en2,start_level;
    double err;
    int* model_vertices;
    List Instances;
    AdjazenzMatrix* MA;
    int current_model;
    Hash used(20);

    d=0;
    MA=&Models_in_tree[the_state[0]->models[0]]->AM;

    current_model=0;
 


    // it is possible that several the_state[x] entries are identical for different x
    // therefore to guarantee the function of getnext, we must create separate
    // Lists foir duplicates!

    List** sequences;
    sequences=new List*[parts];
    List* temp;
    int* created;
    created= new int[parts];

    int min_count=100000;
    int min_x=-1;

    models_checked=0;

    for(x=0;x<parts;x++){
	sequences[x]=NULL;
	for(y=0;y<x;y++)
	    if(sequences[y]==&the_state[x]->sequences){
		sequences[y]=new List;
		(*sequences[y])=the_state[x]->sequences;
		created[x]=1;
		break;
	    }
	if(sequences[x]==NULL){
	    created[x]=0;
	    sequences[x]=&the_state[x]->sequences;
	}
	if(sequences[x]->count()<min_count){
	    min_x=x;
	    min_count=sequences[x]->count();
	}
    }

    if((min_x>0)&&(the_state[0]->depth==the_state[min_x]->depth)){
	gmt_assert(the_state[0]->depth==the_state[min_x]->depth);
	int tempi;
	DT_node* temps;

	temps=the_state[0];
	the_state[0]=the_state[min_x];
	the_state[min_x]=temps;


	temp=sequences[0];
	sequences[0]=sequences[min_x];
	sequences[min_x]=temp;
	for(x=0;x<the_state[0]->depth;x++){
	    tempi=vertex_sequence[start_pos[0]+x];
	    vertex_sequence[start_pos[0]+x]=vertex_sequence[start_pos[min_x]+x];
	    vertex_sequence[start_pos[min_x]+x]=tempi;
	}
    }

    int max_count=ENV_PRUNED_MAX_COUNT;


    for(x=1;x<parts;x++){
#ifdef USE_ULLMAN
	break;
#endif
	if(sequences[x]->count()>max_count)
	    break;
	if(sequences[x]->count()==0)
	    break;
    }

    int real_parts=parts;
    if(x<parts)
	parts=x;



    // model loop

    int *model_index;
    int* seq_count;
    int m;

    seq_count=new int[parts];
    model_index=new int[parts];

    model_vertices=NULL;

    for(m=0;m<=the_state[0]->models_count;m++){
	d=0;
	current_model=the_state[0]->models[m];
	model_index[0]=m;

	for(x=1;x<parts;x++){ 
	    model_index[x]=-1;
	    for(y=0;y<=the_state[x]->models_count;y++)
		if(the_state[x]->models[y]==current_model)
		    model_index[x]=y;
	    if(model_index[x]==-1)
		break;
	}

	if(x<parts) continue;  // no instances of the current_model

	models_checked++;

	MA=&Models_in_tree[the_state[0]->models[m]]->AM;
	if((m>0)&&(model_vertices)) 
	    delete [] model_vertices;

	model_vertices=new int[MA->numberOfVertices()];	

	// advance pointer in sequence to correct position
	sequences[d]->reset();
	seq_count[d]=0;
	for(x=0;x<model_index[d];x++)
	    for(y=0;y<the_state[d]->inst_count[x];y++)
		sequences[d]->getnext();
	// end advance


	while(d>=0){



	    seq[d]=(INT_TYPE*) sequences[d]->getnext();
	    seq_count[d]++;




	    if(seq_count[d]>the_state[d]->inst_count[model_index[d]]){
		d--;
		continue;
	    }

	    if(seq[d]==NULL){
		d--;
		continue;
	    }

	
	    good=1;
	    for(x=0;x<d;x++){
		// check consistency
		for(i=0;i<the_state[x]->depth;i++){
		    for(j=0;j<the_state[d]->depth;j++){
			if(seq[x][i]==seq[d][j]){ 
			    good=0;
			    break;
			}
			
			// check edges!
			
			en1=AM->isEdge(vertex_sequence[start_pos[x]+i],vertex_sequence[start_pos[d]+j]);
			
			en2=MA->isEdge(seq[x][i],seq[d][j]);
			
			if(en1==-1){
			    if(en2!=-1){
				good=0;break;
			    }
			    continue;
			}
			
			if(en2==-1){
			    if(en1!=-1){
				good=0;break;
			    }
			    continue;
			}
			
			err=ATT_object.error(MA->EdgeAttributes[en2].edgeAtt,AM->EdgeAttributes[en1].edgeAtt);
			
			if(err>0){
			    good=0;
			    break;
			}
			
		    }
		    if(good==0) break;
		}
		if(good==0) break;
	    }
	    
	    if(good==0) continue;

	    d++;
	    if(d==parts){
		used.clear();
		start_level=0;
		
		for(x=0;x<d;x++){
		    for(i=0;i<the_state[x]->depth;i++){
			model_vertices[start_level++]=seq[x][i];
			used.insert(seq[x][i]);
		    }
		}
		
		if(start_level<MA->numberOfVertices()){
		    y=start_level;
		    for(x=0;x<MA->numberOfVertices();x++)
			if(!used.in(x))
			    model_vertices[y++]=x;
		    
		    
		}
		
#ifdef USE_ULLMAN
		if(depthFirstTreeSearch(AM,MA,vertex_sequence,model_vertices,start_level,Instances)){
		    MappingData* MP;

		    Instances.reset();
		    while(Instances.count()){
			MP=(MappingData*) Instances.remove(0);
			Collection.insert(MP,current_model,0);
		    }
		    cout << "Success\n";
		}
#else

		if(depthFirstTreeSearch(AM,MA,vertex_sequence,model_vertices,start_level,Instances)){
		    
		    
		    MappingData* MP;
		   		    
		    Token* tok1=new Token(Dim);
		    Token* tok2=new Token(Dim);
		    
		    for(y=0;y<start_level;y++){
			tok1->set(y,model_vertices[y]);
			tok2->set(y,vertex_sequence[y]);
		    }
	   
		    if(start_level==Dim){
			MP = new MappingData;
			MP->setNumber(current_model);
			MP->setMatch(tok2,tok1);
			Collection.insert(MP,current_model,0);
		    }else{
			Instances.reset();
			while(Instances.count()){
			    int* rest=(int*) Instances.remove(0);
			    for(y=start_level;y<Dim;y++){
				tok1->set(y,rest[y-start_level]);
				tok2->set(y,vertex_sequence[y]);
			    }
			    delete [] rest;
		
			    
			    MP = new MappingData;
			    MP->setNumber(current_model);
			    MP->setMatch(tok2,tok1);
			    Collection.insert(MP,current_model,0);
		
			}
		    }
		    delete tok1;
		    delete tok2;
		
		    cout << "Success\n";
		}
#endif
		d--;
	    }else{
		seq_count[d]=0;
		sequences[d]->reset();
		for(x=0;x<model_index[d];x++)
		    for(y=0;y<the_state[d]->inst_count[x];y++)
			sequences[d]->getnext();
	    }

#ifdef USE_ULLMAN
	    break;
#endif


	}
    }
    
    // delete all Lists which were newly created!
    
    for(x=0;x<real_parts;x++)
	if(created[x])
	    delete sequences[x];

    if(model_vertices)
	delete [] model_vertices;
    
    delete [] model_index;
    delete [] seq_count;
    delete [] sequences;
    delete [] created;
    delete seq;
    

}


void 
PG_class::searchNecessaryStates(List& States, DT_node* state){

    int x;


    if(state==NULL) return;
    
    

    if(state->tag>0){
	States.insert(state);
	return;
    }else if(state->allocated==-1) return;
    
    
    for(x=0;x<state->succ;x++){
	searchNecessaryStates(States, state->successors[x]);
    }

}

void 
PG_class::setDrawFunc(void (*func(int,int))){

    draw_func=func;

}



int
PG_class::confirmValidStates(){
    DT_node* state;
    int x,l;

    state=RootNode;
    state->mark=30000;
    state->tag=1;
	
    x=0;
    l=0;
    valid_states=0;
    
    while(recursiveConfirm(state,0,l)) {
	l++;    
	
    }
 
    return 1;
}


int
PG_class::recursiveConfirm(DT_node* state,int level,int limit){
    int s=0;
    int x;
    DT_node* hstate;

    if(state==NULL) return 0;
 
    if(limit==0){       
       return 1;
     }

    if((level==limit-1)){

	if(state->allocated==-1) return 1;

	if(state->mark==30000) {
	    for(x=0;x<state->succ;x++){
		hstate=state->successors[x];
		
		if((hstate)&&(hstate->tag>0)){
		    hstate->mark=30000;
		
		    while(hstate->allocated==-1){
			hstate=hstate->successors[0];
			if(hstate->tag>0) hstate->mark=30000;
		    }
		}
	    }
	}
	return 1;
    }else{
	int r=0;
	
	if(state->allocated==-1) return 0;
	for(x=0;x<state->succ;x++){
	  r+=recursiveConfirm(state->successors[x],level+1,limit);
	}
	return r;
    }
}


void
PG_class::mergeWithDistortions(Graph* G,int num){
  Edit_type* E;
  Graph* G2;
  int x,key,i,orig;
  int* v;
  SortedList listOfDistortions;
  INT_TYPE* v_seq;


  AuxilaryModels.clear();

  // original graph
    
  orig=num;
  merge1(G,num);

  // PARAMETERS indictaes in this case the total number of inserted or deleted
  // edges that are to be considered for a single model graph. -> combinations

  x=num+1;
  v_seq=new INT_TYPE[G->numberOfVertices()];
  G->generateDistortions(listOfDistortions,PARAMETERS);
  //while(listOfDistortions.count()>0){
  
  Graph* G1=G;
  int *cs;
  int y,er,index;
  List del_list;

  if(PARAMETERS>listOfDistortions.count())
    PARAMETERS=listOfDistortions.count();

  cs=new int[(int) PARAMETERS];

  for(er=1;er<=PARAMETERS;er++){
    for(y=0;y<er;y++) cs[y]=y;
    
    index=er;
    while(index>=0){
    
      
      G2=NULL;
      G1=G;

      for(y=0;y<er;y++){

	if(G2) del_list.insert(G2);

	E=(Edit_type*) listOfDistortions.get(cs[y],key);
	
	G2=G1->applyEdits(E);
	AM=&G2->AM;
	Dim=G2->numberOfVertices();


	for(i=0;i<G2->numberOfVertices();i++)
	  v_seq[i]=i;
	
	if(!orderInLine(v_seq,1,G2->numberOfVertices(),G2->numberOfVertices())){
	  delete G2;	  	
	  G1=NULL;
	  break;
	}else{
	  G1=G2;
	}
      }

      while(del_list.count())
	delete (Graph*) del_list.remove(0);
      

      if(G1){
	AuxilaryModels.insert(G1);
	merge1(G1,x);
	v=new int;
	*v=orig;
	OriginalModels.insert(v,x);
	x++;    
      }
      
      index=er-1;
      while(index<er){
	cs[index]++;
	if(cs[index]>=listOfDistortions.count()){
	  index--;
	  if(index<0) break;
	}else{
	  index++;
	  if(index<er) cs[index]=cs[index-1];
	}
      }
    }
  }

  delete [] v_seq;
}



int
PG_class::recognitionWithDistortions(Graph* G){
  Edit_type* E;
  Graph* G1,*G2;
  int x,key,i,orig;
  int* v;
  SortedList listOfDistortions;
  int found=0;
  SortedList temp;
  MappingData* mp;
  double val;
  int *cs;
  int y,er,index;
  List del_list;
  INT_TYPE* v_seq;

  OriginalAM=&G->AM;

 
  // PARAMETERS indictaes in this case the total number of inserted or deleted
  // edges that are to be considered for a single model graph. -> combinations

 
  if(isomorph1(G)){
    found=1;
    while(Collection.count()){
      val=Collection.TopValue();
      mp=(MappingData*) Collection.removeTop(&key);
      temp.insert(mp,key,val);
    }
  }  


  v_seq=new INT_TYPE[G->numberOfVertices()];
  G->generateDistortions(listOfDistortions,PARAMETERS);
  //while(listOfDistortions.count()>0){
  
 
  if(PARAMETERS>listOfDistortions.count())
    PARAMETERS=listOfDistortions.count();

  cs=new int[(int) PARAMETERS];

  for(er=1;er<=PARAMETERS;er++){
    for(y=0;y<er;y++) cs[y]=y;
    
    index=er;
    while(index>=0){
    
      
      G2=NULL;
      G1=G;

      for(y=0;y<er;y++){

	if(G2) del_list.insert(G2);

	E=(Edit_type*) listOfDistortions.get(cs[y],key);
	
	G2=G1->applyEdits(E);
	AM=&G2->AM;
	Dim=G2->numberOfVertices();


	for(i=0;i<G2->numberOfVertices();i++)
	  v_seq[i]=i;
	
	if(!orderInLine(v_seq,1,G2->numberOfVertices(),G2->numberOfVertices())){
	  delete G2;     	
	  G2=NULL;
	  break;
	}else{
	  G1=G2;
	}
      }

      while(del_list.count())
	delete (Graph*) del_list.remove(0);
      

      if(G2){
	if(isomorph1(G2)){
	  found=1;
	  while(Collection.count()){
	    val=Collection.TopValue();
	    mp=(MappingData*) Collection.removeTop(&key);
	    temp.insert(mp,key,val);
	  }
	}  
	delete G2;
      
      }

      
      index=er-1;
      while(index<er){
	cs[index]++;
	if(cs[index]>=listOfDistortions.count()){
	  index--;
	  if(index<0) break;
	}else{
	  index++;
	  if(index<er) cs[index]=cs[index-1];
	}
      }
    }
  }

  delete [] v_seq;


  //////


 
  while(temp.count()){
     val=temp.TopValue();
     mp=(MappingData*) temp.removeTop(&key);
     Collection.insert(mp,key,val);
  }

  return found;
}



void
PG_class::auxilaryModels(List& l){

  AuxilaryModels.mv(l);

}
