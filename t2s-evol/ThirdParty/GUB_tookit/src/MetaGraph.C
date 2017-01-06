#include"MetaGraph.h"
#include"Learn.h"

static int SUBSTRUCTURE_MAX_SIZE=5;
static int SUBSTRUCTURE_MIN_SIZE=0;
static int ENV_CUTOFF_SUBDUE=100;

In_MetaGraph* hierarchy4(In_MetaGraph*& M);
In_MetaGraph* hierarchy3(In_MetaGraph* M);
In_MetaGraph* hierarchy2(In_MetaGraph*& M);
In_MetaGraph* hierarchy(In_MetaGraph* M);
List* compress1(List* Gl);

extern double edgeErrorBetween2Vertices(AdjazenzMatrix* WModel, AttributeClass* ATTC, AttId* e_l,int dim_l,int l,int r,int dir);


inline void MetaGraph::deleteThis(MetaGraph* pt){
  
  if(pt->referenced==0)
    delete pt;
  else
    pt->referenced--;

}


/* Member functions of MetaGraph */

MetaGraph::MetaGraph(){
  represented=0;
  referenced=0;
}

MetaGraph::~MetaGraph(){
 
  gmt_assert(referenced==0);
    
}


/* Member functions of In_MetaGraph   */

In_MetaGraph::In_MetaGraph(){
   tok=new Token;
}
  

In_MetaGraph::~In_MetaGraph(){
 delete tok;
 In_MetaGraph* M;

 while(parts.count()>0){
   M=(In_MetaGraph*) parts.remove(0);
   delete M;
 }
 
 MetaGraph::deleteThis(MyGraph);

}

void 
In_MetaGraph::init(MetaGraph* M,Graph* G){

  MyGraph=M;
  M->referenced++;
  graph=G;
 
}


void 
In_MetaGraph::init(int v,MetaGraph* M,Graph* G){

  graph=G;
  MyGraph=M;
  M->referenced++;
  vertex=v;

  delete tok;
  tok=new Token(1);
  tok->set(0,v);

}


void 
In_MetaGraph::add(In_MetaGraph *M){
  Token* htok;
  int x,i;

  htok=tok;

  tok=new Token(htok->length()+M->tok->length());
  
  for(x=0;x<htok->length();x++)
    tok->set(x,(*htok)[x]);
  
  i=htok->length();
  for(x=0;x<M->tok->length();x++)
    tok->set(i+x,(*M->tok)[x]);

  parts.insert(M);
  
}


void
In_MetaGraph::getVertices(List& l){
  In_MetaGraph* m;

  if(parts.count()>0){
    parts.reset();
    while(m=(In_MetaGraph*) parts.getnext())
      m->getVertices(l);    
  }else{
    l.insert(&vertex);
  }
}


int
In_MetaGraph::connectedTo(In_MetaGraph* M){
  int x,y; 
  int d;
  d=0;
  for(x=0;x<tok->length();x++)
    for(y=0;y<M->tok->length();y++){
      if(graph->AM.isEdge((*tok)[x],(*M->tok)[y])>-1) d++;
      if(graph->AM.isEdge((*M->tok)[y],(*tok)[x])>-1) d++;
    }
  
  return d;
  
}

int
In_MetaGraph::size(){

  if(tok)
    return tok->length();
  else 
    return 0;
}


int 
In_MetaGraph::degree(int& inner){
  Hash ht;
  int x,v,d;
  int out,in;
  AttId at;

  inner=0;
  if(tok==NULL) return 0;

  for(x=0;x<tok->length();x++){
    v=(*tok)[x];
    ht.insert(NULL,v);
  }

  d=0;
  for(x=0;x<tok->length();x++){
    v=(*tok)[x];
    graph->AM.initNext(v);
    while(graph->AM.getnext(&in,&out,&at)>-1){
      if(in==v){
	if(!ht.in(out)) d++;
	else inner++;
      }else{
	if(!ht.in(in)) d++;
	else inner++;
      }
    }
  }
  return d;
}


int 
In_MetaGraph::innerDegree(){
  Hash ht;
  int x,v,d;
  int out,in;
  AttId at;

  if(tok==NULL) return 0;

  for(x=0;x<tok->length();x++){
    v=(*tok)[x];
    ht.insert(NULL,v);
  }

  d=0;
  for(x=0;x<tok->length();x++){
    v=(*tok)[x];
    graph->AM.initNext(v);
    while(graph->AM.getnext(&in,&out,&at)>-1){
      if(in==v){
	if(ht.in(out)) d++;
      }else{
	if(ht.in(in)) d++;
      }
    }
  }
}



void
In_MetaGraph::clear(){
  delete tok;
  tok=new Token;
  parts.clear();
}


Graph*
In_MetaGraph::subGraph(){
  Graph* G;
  int *v,x;
  List gv;

  for(x=0;x<tok->length();x++){
    v=new int;
    *v=(*tok)[x];
    gv.insert(v);
  }

  G=graph->getSubgraph(gv);

  gv.clearDeep();
  
  return G;
}



/* end member functions */

List* createMetaGraph(List& graph_list, int type){
  Graph* G;
  In_MetaGraph* M,*M1,*M2;
  List* Ml;
  List Ml1;
  int LINEAR_COMPILE=0;
  int x,y;

  char* heu;
  if(heu=getenv("ENV_MAX_SUBSIZE")) SUBSTRUCTURE_MAX_SIZE=atoi(heu);
  if(heu=getenv("ENV_MIN_SUBSIZE")) SUBSTRUCTURE_MIN_SIZE=atoi(heu);
  if(heu=getenv("ENV_LINEAR")) LINEAR_COMPILE=1;
  if(heu=getenv("ENV_CUTOFF_SUBDUE")) ENV_CUTOFF_SUBDUE=atoi(heu);


  if(type>=4)
    SUBSTRUCTURE_MAX_SIZE=type-4;


  if((LINEAR_COMPILE)||(type==3)){
    Token* tok;  
    
    Ml=compress1(&graph_list);
    Ml1=(*Ml);
    Ml->clear();

    for(y=0;y<graph_list.count();y++){
      G=(Graph*) graph_list.get(y);
      tok=G->AM.orderCoherent();
      M=(In_MetaGraph*) Ml1.get(y);
      M1=new In_MetaGraph;
      M1->init(M->MyGraph,M->graph);
      for(x=tok->length()-1;x>=0;x--){
	M2=(In_MetaGraph*) M->parts.get((*tok)[x]);
	M1->add(M2);
      }
      M1=hierarchy(M1);
      
      Ml->insert(M1);
    }
    return Ml;

  }else{
    
    Ml=compress(&graph_list);
        
    cout << "Hierarchical decomposition started...\n";
    
    int x;
    Ml->mv(Ml1);
    for(x=0;x<Ml1.count();x++){
      M=(In_MetaGraph*) Ml1.get(x);
      hierarchy4(M);
      Ml->insert(M);
    }

    return Ml;
  }
}







/*********************************************/
/* COMPRESS AND HIERARCHY PROCEDURES         */
/*********************************************/

/* currently available compres functions: */

List* compress2(List* Ml);

List* compress(List* Gl){

  return compress2(Gl);
 
}


In_MetaGraph* hierarchy2(In_MetaGraph*& M){
// divide randomly in two parts
  In_MetaGraph* H;
  MetaGraph* MG;
  int x,y,dim;
  In_MetaGraph** mv;
  double **m,**m0;
  Graph *G1,*G2;
  Graph** gv;
  List g_list;
  int alive;
  int *dead;
  double best;
  int best_x,best_y;
  
 
  dim=M->parts.count();
  gmt_assert(dim);
  alive=dim;
  
  dead=new int[dim];
  mv=new In_MetaGraph*[dim];
  gv=new Graph*[dim];
  for(x=0;x<dim;x++){
    dead[x]=0;
    mv[x]=(In_MetaGraph*) M->parts.get(x);
    gv[x]=mv[x]->subGraph();
    g_list.insert(gv[x]);
  }

  // treat all MetaGraphs in mv

  for(x=0;x<dim;x++)
    if(mv[x]->parts.count()>0){
      In_MetaGraph* mv2;
      mv2=mv[x];
      mv[x]=hierarchy3(mv[x]);
    
    }

  //init distance matrix
  
  if(dim>1){
    
#if 0
    m=distanceBetweenGraphs(&g_list,4.0);
#else
    m=new double*[dim];
    for(x=0;x<dim;x++){
      m[x]=new double[dim];
      for(y=0;y<dim;y++){
	if(x==y) continue;
	m[x][y]=abs(gv[x]->numberOfVertices()+gv[y]->numberOfVertices());
      }
    }
#endif
    
    for(x=0;x<dim;x++){
      for(y=0;y<dim;y++){
	if(x==y) continue;
	if(!mv[x]->connectedTo(mv[y]))
	  m[x][y]+=INFTY_COST+1;
      }
    }
    // collect clusters
    
     
    while(alive>1){
      best=3*INFTY_COST;
      for(x=0;x<dim;x++){
	if(dead[x]) continue;
	for(y=0;y<dim;y++){
	  if((x==y)||(dead[y])) continue;
	  if(m[x][y]<best){
	    best_x=x;
	    best_y=y;
	    best=m[x][y];
	  }
	}
      }
      
      H=new In_MetaGraph;
      MG=new MetaGraph;
      H->init(MG,mv[best_x]->graph);
      H->add(mv[best_x]);
      H->add(mv[best_y]);
      mv[best_x]=H;
      delete gv[best_x];
      delete gv[best_y];
      gv[best_x]=H->subGraph();
      dead[best_y]=1;
      alive--;
      
      // update row best_x, dummy, cheaply
      
      for(x=0;x<dim;x++){
	if((x==best_x)||(x==best_y)) continue;
	if(!dead[x]){
#if 0
	  g_list.clear();
	  g_list.insert(gv[best_x]);
	  g_list.insert(gv[x]);
	  m0=distanceBetweenGraphs(&g_list,4.0);
	  
	  m[best_x][x]=m0[0][1];
	  m[x][best_x]=m0[1][0];
	  
	  if(!mv[best_x]->connectedTo(mv[x])){
	    m[best_x][x]+=INFTY_COST+1;
	    m[x][best_x]+=INFTY_COST+1;
	  }
	  delete[] m0;
#else
	  m[best_x][x]=abs(gv[best_x]->numberOfVertices()+gv[x]->numberOfVertices());
	  m[x][best_x]=m[best_x][x];
	  if(!mv[best_x]->connectedTo(mv[x])){
	    m[best_x][x]+=INFTY_COST+1;
	    m[x][best_x]+=INFTY_COST+1;
	  }
	  
#endif
	}
      }
    }

    for(x=0;x<dim;x++) delete m[x];
    delete[] m;

    delete gv[best_x];
    M->clear();
    delete M;
    M=H;
  }


  delete dead;
  delete mv;
  delete gv;

  
  return H;

}

In_MetaGraph* hierarchy3(In_MetaGraph* M){

// simple linear compilation
// this decomposition MUST be reversible, such that for isomorph structures
// it will produce the same result!

  In_MetaGraph* H,*H1,*H2,*M1,*M2,*H3,*H4,*H5;
  MetaGraph* MG;
  int x;
  
  if(M->parts.count()==0)
    return M;
  
  if(M->parts.count()==1){
    H=hierarchy3((In_MetaGraph*) M->parts.get(0));
    H->MyGraph=M->MyGraph;
    H->MyGraph->referenced++;
    return H;
  }

  H=new In_MetaGraph;
  
  H->init(M->MyGraph,M->graph);         

  H1=(In_MetaGraph*) M->parts.remove(0);
  H2=hierarchy3(H1);
  while(M->parts.count()>0){
    H4=NULL;
    for(x=0;x<M->parts.count();x++){
      H3=(In_MetaGraph*) M->parts.get(x);
      if(H2->connectedTo(H3)){
	H4=(In_MetaGraph*) M->parts.remove(x);
	break;
      }
    }
    gmt_assert(H4!=NULL);
    
    H5=hierarchy3(H4);
    H=new In_MetaGraph;
    MG=new MetaGraph;
    H->init(MG,M->graph);
    
    H->add(H2);
    H->add(H5);

    H2=H;
  }
  
  H2->MyGraph=M->MyGraph;
  H2->MyGraph->referenced++;
  delete M;
  return H2;
    
}


In_MetaGraph* hierarchy(In_MetaGraph* M){
// divide randomly in two parts
  In_MetaGraph* H,*H1,*H2,*M1,*M2,*H3;
  MetaGraph* MG;
  int x;

  // this function backtracks if a division is found which
  // has no edges inbetween!


  if(M->parts.count()==0)
    return M;


  if(M->parts.count()==1)
    return hierarchy((In_MetaGraph*) M->parts.get(0));

  H=new In_MetaGraph;
  
  H->init(M->MyGraph,M->graph);                    // use old MetaGraph for H

  // first part (left parent)

  int backtrack=1;
  int i;
  i=-1;

  MG=new MetaGraph;
  H2=new In_MetaGraph;
  H2->init(MG,M->graph);    
  
  
  while(backtrack){
    i++;
    if(i==M->parts.count()) break;
    H2->clear();
    // left parent

    M1=(In_MetaGraph*) M->parts.get(i);

    // second part (right parent)
    
    for(x=0;x<M->parts.count();x++){
      if(x==i) continue;
      M2=(In_MetaGraph*) M->parts.get(x);                     
      H2->add(M2);
    }
    
    if(!M1->connectedTo(H2)) continue;

    M2=hierarchy(H2);                              // recursive call

    if(M2!=NULL) 
      backtrack=0;
  }
  
  if(backtrack) return NULL;

  H1=hierarchy(M1);                                // recursive call
  
  if(H1==NULL) return NULL;

  H->add(H1);
  
  H->add(M2);
    
  return H;
}



List* compress1(List* Gl){
  // one level compression (only vertices)
  int x,vertex,v;
  In_MetaGraph* M,*Mi;
  MetaGraph* MGi,*ref_MGi;
  List Labels;
  AttId at0,at1;
  List* Ml;
  Graph* G;

  Ml=new List;


  Gl->reset();
  while(G=(Graph*) Gl->getnext()){
    M=new In_MetaGraph;
    MGi=new MetaGraph;
    M->init(MGi,G);
    
    for(x=0;x<G->numberOfVertices();x++){
      Mi=new In_MetaGraph;
      
      at0=G->AM.getNodeAttributeId(x);
      Labels.reset();
      ref_MGi=NULL;
      while(MGi=(MetaGraph*) Labels.getnext()){
	v=(*MGi->original)[0];
	at1=MGi->graph->AM.getNodeAttributeId(v);
	
	if(ATT_object.error(at0,at1)==0){
	  ref_MGi=MGi;
	  break;
	}
      }
      if(ref_MGi==NULL){
	ref_MGi=new MetaGraph;
	ref_MGi->graph=G;
	ref_MGi->original=new Token(1);
	ref_MGi->original->set(0,x);
	Labels.insert(ref_MGi);
      }
      
      
      Mi->init(x,ref_MGi,G);
      
      M->add(Mi);
      
    }
    Ml->insert(M);
  }
  return Ml;
}


struct O_el{
  O_el* up;
  int v;
  List instances;
};



int checkEdges2(In_MetaGraph** mv,int l,int k,O_el* o,O_el* o1,In_MetaGraph** V){
  if(o1->v==-1) return 1;
  AttId e_l[10],at;
  int dim_l,edg,x,y;
  Token *t1,*t2,*t3,*t4;

  t1=mv[l]->tok;
  t2=mv[k]->tok;

  t3=V[o->v]->tok;
  t4=V[o1->v]->tok;

  gmt_assert(t1->length()==t3->length());
  gmt_assert(t2->length()==t4->length());
  
  

  for(x=0;x<t1->length();x++){
    for(y=0;y<t2->length();y++){
      mv[l]->graph->AM.initNext((*t1)[x],(*t2)[y]);
      
      dim_l=0;
      while((at=mv[l]->graph->AM.isNextEdgeAttId(&edg))!=NO_ATTRIBUTE)
	e_l[dim_l++]=at;
      e_l[dim_l++].set(-1,NULL,0);
      
      if(edgeErrorBetween2Vertices(&V[o->v]->graph->AM, &ATT_object, e_l,dim_l, (*t3)[x], (*t4)[y], 1)>0) return 0;
      
      dim_l=0;
      mv[l]->graph->AM.initNext((*t2)[y],(*t1)[x]);
      
      while((at=mv[l]->graph->AM.isNextEdgeAttId(&edg))!=NO_ATTRIBUTE)
	e_l[dim_l++]=at;
      e_l[dim_l++].set(-1,NULL,0);
      
      if(edgeErrorBetween2Vertices(&V[o->v]->graph->AM, &ATT_object, e_l,dim_l,(*t3)[x], (*t4)[y] , -1)>0) return 0;
      
    }
  }

  return checkEdges2(mv,l,k-1,o,o1->up,V);
}



int checkEdges(int *vertices,int l,int k,O_el* o,O_el* o1,In_MetaGraph** V){
  if(o1->v==-1) return 1;
  AttId e_l[10],at;
  int dim_l,edg;


  dim_l=0;
  V[l]->graph->AM.initNext(vertices[l],vertices[k]);
  while((at=V[l]->graph->AM.isNextEdgeAttId(&edg))!=NO_ATTRIBUTE)
    e_l[dim_l++]=at;
  e_l[dim_l++].set(-1,NULL,0);

  if(edgeErrorBetween2Vertices(&V[l]->graph->AM, &ATT_object, e_l,dim_l, o->v, o1->v, 1)>0) return 0;
  
  dim_l=0;
  V[l]->graph->AM.initNext(vertices[k],vertices[l]);
  while((at=V[l]->graph->AM.isNextEdgeAttId(&edg))!=NO_ATTRIBUTE)
    e_l[dim_l++]=at;
  e_l[dim_l++].set(-1,NULL,0);

  if(edgeErrorBetween2Vertices(&V[l]->graph->AM, &ATT_object, e_l,dim_l, o->v, o1->v, -1)>0) return 0;

  return checkEdges(vertices,l,k-1,o,o1->up,V);
}


double evaluateStructure2(int* current,int current_x,int ones,int* dim,int current_i,In_MetaGraph*** V,List& instances){
  In_MetaGraph** mv;
  double eval;
  int x,y,l,z,size;
  O_el* o,*o2,*root;
  SortedList OPEN;
  List CLOSED;

  // match the substructure defined by o1 onto M
  // use a simple tree search
  Token *tok;
  
  y=0;size=0;
  mv=new In_MetaGraph*[ones];
  int d=0;
  for(x=0;x<dim[current_x];x++)
    if(current[x]){
      mv[y++]=V[current_x][x];
      size+=mv[y-1]->size();
      for(z=0;z<y-1;z++){
	d+=mv[z]->connectedTo(mv[y-1]);
      }
    }
//  if((size>3)&&(d/2<ones)) return dim[current_i]+ones;
  if((size>3)&&(d/2<ones)) return 0;
    
  eval=0;
 
  
  root=new O_el; root->v=-1;  root->up=NULL;
  OPEN.insert(root,-1,-1);
  while(OPEN.count()>0){
    
    o=(O_el*) OPEN.removeTop(&l);
    
    if(l==ones-1){
      // instances found
      int k;
      eval++;
      tok=new Token(ones);
      o2=o;
      k=l;
      while(o2->v!=-1){
	tok->set(k--,o2->v);
	o2=o2->up;
      }
      instances.insert(tok);
      delete o;
      continue;
    }
      
    for(x=0;x<dim[current_i];x++){
      o2=o;
      while(o2){
	if(o2->v==x) break;
	o2=o2->up;
      }
      if(o2) continue;

      if(mv[l+1]->MyGraph!=V[current_i][x]->MyGraph) continue;
     
      o2=new O_el;
      o2->v=x;
      o2->up=o;
      if(checkEdges2(mv,l+1,l,o2,o2->up,V[current_i])){
	OPEN.insert(o2,l+1,l+1);
      }else{
	delete o2;
      }
    }
    CLOSED.insert(o);
  }

  while(CLOSED.count()>0){
    o=(O_el*) CLOSED.remove(0);
    delete o;
  }

 // determine largest clique of instances
  
  int i=instances.count();
  int **mx=new int*[i];
  Token** K;
  int* Current,*Best;
  int Currenti,Besti;

  K=new Token*[i];
  for(x=0;x<i;x++){
    mx[x]=new int[i];
    K[x]=(Token*) instances.get(x);
  }
  
  for(x=0;x<i;x++){ 
    for(y=0;y<i;y++){
      if(x==y){
	mx[x][y]=1;
	continue;
      }
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

  instances.clear();
  for(x=0;x<i;x++)
    if(Best[x]) instances.insert(K[x]);
    else delete K[x];

  delete Best;
  delete Current;
  for(x=0;x<i;x++) delete mx[x];
  delete mx;
  delete K;

//   eval=instances.count();
//   eval=dim[current_i]-eval*ones+eval+ones;
//   return eval;
  return instances.count();
  
}


double evaluateStructure(O_el* o1,In_MetaGraph *M, List& instances,In_MetaGraph** V,int gdim){
  double eval;
  int* vertices;
  int x,y,dim,l;
  O_el* o,*o2,*root;
  SortedList OPEN;

  // match the substructure defined by o1 onto M
  // use a simple tree search
  Token *tok;
  
  dim=0;
  eval=0;
  o=o1;
  while(o){dim++;o=o->up;}
  vertices=new int[dim];
  o=o1;
  x=0;
  while(o){vertices[x++]=o->v;o=o->up;}
  
  
  root=new O_el;
  root->v=-1;
  root->up=NULL;
  OPEN.insert(root,-1,-1);

  while(OPEN.count()>0){
    
    o=(O_el*) OPEN.removeTop(&l);
    
    if(l==dim-1){
      // instances found
      int k;
      eval++;
      tok=new Token(dim);
      o2=o;
      k=l;
      while(o2->v!=-1){
	tok->set(k--,o2->v);
	o2=o2->up;
      }
      instances.insert(tok);
      continue;
    }
      
    for(x=0;x<gdim;x++){
      o2=o;
      while(o2){
	if(o2->v==x) break;
	o2=o2->up;
      }
      if(o2) continue;

      if(V[vertices[l+1]]->MyGraph!=V[x]->MyGraph) continue;
     
      o2=new O_el;
      o2->v=x;
      o2->up=o;
      if(checkEdges(vertices,l+1,l,o2,o2->up,V)){
	OPEN.insert(o2,l+1,l+1);
      }else{
	delete o2;
      }
    }
  }


  // determine largest clique of instances
  
  int i=instances.count();
  int **mx=new int*[i];
  Token** K;
  int* Current,*Best;
  int Currenti,Besti;

  K=new Token*[i];
  for(x=0;x<i;x++){
    mx[x]=new int[i];
    K[x]=(Token*) instances.get(x);
  }
  
  for(x=0;x<i;x++){ 
    for(y=0;y<i;y++){
      if(x==y){
	mx[x][y]=1;
	continue;
      }
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

  instances.clear();
  for(x=0;x<i;x++)
    if(Best[x]) instances.insert(K[x]);
    else delete K[x];

  delete Best;
  delete Current;
  for(x=0;x<i;x++) delete mx[x];
  delete mx;
  delete K;

  eval=instances.count();
  eval=gdim-eval*dim+eval+dim;
  return eval;
}


double subDue(int col,int ones,In_MetaGraph ***V,int *current,int* dim,List* instances,int* best,double& eval_global,int& global_ones, int& current_x,int& numOfModels,int& Zdim){
  int x,y,i,z;
  double eval,eval2,eval1,val;
  List *instances1;
  static int stopSearch=0;
  int *best2;
  int connected;

  stopSearch=0;

  if(ones>SUBSTRUCTURE_MAX_SIZE) return Zdim+1;
  if(col==dim[current_x]) return Zdim+1;

  instances1=new List[numOfModels];

  eval1=-1;

  Token *tok1,*tok2;
  int prevDone=0;
  
  if(ones>0){
    tok1=new Token(ones+1);
    z=0;
    for(y=0;y<col;y++)
      if(current[y]) tok1->set(z++,y);
  }

  for(x=col;x<dim[current_x];x++){
    
    connected=0;
    for(y=0;y<col;y++)
      if(current[y])
	if(V[current_x][y]->connectedTo(V[current_x][x])){
	  connected=1;
	  break;
	}
    
    if(ones>0)
      if(connected==0) continue;


    current[x]=1;    

    
    if(ones>SUBSTRUCTURE_MIN_SIZE){
      eval=0;
      for(i=0;i<numOfModels;i++){
	
// is current instances part of previous instances ?
	
	prevDone=0;
	
	if(ENV_CUTOFF_SUBDUE<100){
	  if(ones>0){ 
	    
	    tok1->set(ones,x);
	    if(instances[current_x].count()>0){
	      instances[current_x].reset();
	      for(z=0;z<instances[current_x].count();z++){
		tok2=(Token*) instances[current_x].getnext();
		if(tok1->length()==tok2->length()){
		  if((*tok1)<(*tok2)){
		    prevDone=1;
		    break;
		}
		} 
	      }
	    }
	  }
	}
	if(prevDone){
	  eval+=dim[i]+(ones+1);
	  continue;
	}

	val=evaluateStructure2(current,current_x,ones+1,dim,i,V,instances1[i]);
	
	eval+=dim[i]-val*(ones+1)+val+(ones+1);
	
	//cut of search heuristically:
	if((val-1)*(ones)>ENV_CUTOFF_SUBDUE)
	  stopSearch=1;

      }

      eval-=(numOfModels-1)*(ones+1);
   
    
      if((stopSearch)||(eval<eval_global)||((eval==eval_global)&&(ones>global_ones))){
	global_ones=ones;
	for(i=0;i<numOfModels;i++){
	  while(instances[i].count()>0) delete (Token*) instances[i].remove(0);
	  instances1[i].mv(instances[i]);
	}
	for(y=0;y<=x;y++) best[y]=current[y];
	for(y=x+1;y<dim[current_x];y++) best[y]=0;
	eval_global=eval;
	
      }else{
	for(i=0;i<numOfModels;i++)
	  while(instances1[i].count()>0)
	    delete (Token*) instances1[i].remove(0);
      }
    }else{
      eval=Zdim;
    }

    if((eval<=Zdim)||(ones<=SUBSTRUCTURE_MIN_SIZE))
      eval=subDue(x+1,ones+1,V,current,dim,instances,best,eval_global,global_ones, current_x, numOfModels, Zdim);
    else
      eval=Zdim+1;

    current[x]=0;    

    if(stopSearch) break;
  }
 
  delete[] instances1;
  return eval_global;
}    


In_MetaGraph* optimalSubstructure2(List* Ml,double& compress_rate){
  MetaGraph *MG;
  In_MetaGraph* M,*M1,*M2,*m;
  int x,y,z,l;
  double eval,current_val,global_eval;
  List *instances,*best_instances;
  In_MetaGraph ***V;
  int* current;
  int* best,*dim,*best_best;
  Token* tok;
  int numOfModels,Zdim;
  int max_dim;

  // map vertices in M onto array
  numOfModels=Ml->count();
  dim=new int[numOfModels];
  instances=new List[numOfModels];
  best_instances=new List[numOfModels];

  V=new In_MetaGraph**[numOfModels];
  Zdim=0;
  max_dim=0;
  for(y=0;y<numOfModels;y++){
    M=(In_MetaGraph*) Ml->get(y);
    dim[y]=M->parts.count();
    V[y]=new In_MetaGraph*[dim[y]];
    for(x=0;x<dim[y];x++)
      V[y][x]=(In_MetaGraph*) M->parts.get(x);

    M->clear();
    Zdim+=dim[y];
    if(max_dim<dim[y]) 
      max_dim=dim[y];
  } 

  current=new int[max_dim];;
  best=new int[max_dim];
  best_best= new int[max_dim];
  
  eval=Zdim+1;
  global_eval=Zdim+1;
  int subsize=0;
  int best_subsize=0;
  for(x=0;x<numOfModels;x++){
    for(y=0;y<max_dim;y++){current[y]=0;best[y]=0;}
    
    subDue(0,0,V,current,dim,instances,best,eval,subsize,x,numOfModels,Zdim);
    if((eval<global_eval)||((eval==global_eval)&&(subsize>best_subsize))){
      best_subsize=subsize;
      global_eval=eval;
      for(z=0;z<numOfModels;z++){
	while(best_instances[z].count()>0) delete (Token*) best_instances[z].remove(0);
	instances[z].mv(best_instances[z]);
	
      }
    }else{
      for(z=0;z<numOfModels;z++)
	while(instances[z].count()>0) delete (Token*) instances[z].remove(0);
    }
  }
  compress_rate=global_eval/Zdim;

  
  MG=new MetaGraph;
  int sizes=-1;
  for(y=0;y<numOfModels;y++){
    M=(In_MetaGraph*) Ml->get(y);
    best_instances[y].reset();
    sizes=-1;
    while(tok=(Token*) best_instances[y].getnext()){
      if(tok->length()!=sizes) gmt_assert(sizes==-1);
      sizes=tok->length();
      
      M1=new In_MetaGraph; 
      M1->init(MG,M->graph);
      
      for(x=0;x<tok->length();x++){
	
	gmt_assert(V[y][(*tok)[x]]!=NULL);
	M1->add(V[y][(*tok)[x]]);
	V[y][(*tok)[x]]=NULL;
	
      }
      
      M->add(M1);
    }
    for(x=0;x<dim[y];x++){
      if(V[y][x]){
	M->add(V[y][x]);
      }
    }

    while(best_instances[y].count()>0) delete (Token*) best_instances[y].remove(0);
    delete V[y];
  }
  
  delete V;
  delete current;
  delete best;
  

  return NULL;

}



In_MetaGraph* optimalSubstructure(In_MetaGraph* Ml){
  SortedList OPEN,CLOSED;
  In_MetaGraph *M1,*M2,*m,*M;
  int x,y,dim,l;
  double eval,current_val;
 
  O_el* o,*o1;
  int proceed;

  // map vertices in M onto array

  In_MetaGraph **V;
  V=new In_MetaGraph*[M->parts.count()];
  dim=Ml->parts.count();
  for(x=0;x<dim;x++)
    V[x]=(In_MetaGraph*) M->parts.get(x);

  
  // init OPEN

  M->parts.reset();
  for(x=0;x<dim;x++){
    o=new O_el;
    o->v=x;
    o->up=NULL;
   
    for(y=0;y<dim;y++){
      if(o->v==y) continue;
      if(M->graph->AM.isEdge(o->v,y)>-1){
	o1=new O_el;
	o1->v=y;
	o1->up=o;
	
	eval=evaluateStructure(o1,M,o1->instances,V,dim);

	OPEN.insert(o1,1,eval);
      }
    }
  }

  // process OPEN until no improvement is found

  while(OPEN.count()>0){
    current_val=OPEN.TopValue();
    if(current_val>dim) break;
    
    o=(O_el*) OPEN.removeTop(&l);
    
    proceed=0;
    for(y=0;y<dim;y++){
      o1=o;
      while(o1){
	if(o1->v==y) break;
	o1=o1->up;
      }
      if(o1) continue;

      if(M->graph->AM.isEdge(o->v,y)>-1){
	o1=new O_el;
	o1->v=y;
	o1->up=o;
	
	eval=evaluateStructure(o1,M,o1->instances,V,dim);
	
	if((current_val<eval)||(eval>dim)){
	  while(o1->instances.count()>0)
	    delete (Token*) o1->instances.remove(0);
	  
	  delete o1;
	}else{
	  
	  OPEN.insert(o1,l+1,eval);
	  proceed=1;
	}
      }
    }
    if(!proceed)
      CLOSED.insert(o,l,current_val);
  }

  while(OPEN.count()>0)
    delete (O_el*) OPEN.removeTop(&l);


  cout << "Creating new MetaGraph\n";

  // build new In_MetaGraph for each instance
  MetaGraph* MG;
  Token* tok;

  o=(O_el*) CLOSED.removeTop(&x);
  
  o->instances.reset();
  //each token in instances can be turned into a In_MetaGraph,
  //pointing to the same MetaGraph reference
  //WATCH the order of the vertices!
  
  MG=new MetaGraph;

  while(tok=(Token*) o->instances.getnext()){

    M1=new In_MetaGraph;
    
    M1->init(MG,M->graph);
    
    for(x=0;x<tok->length();x++){
      M->parts.reset();
      y=0;
      while(m=(In_MetaGraph*) M->parts.getnext()){

	if(m==V[(*tok)[x]]){
	  M->parts.remove(y);
	  M1->add(m);
	  break;
	}
	y++;
      }
    }
    gmt_assert(M1->parts.count()==tok->length());
    
    M->add(M1);
  }
  o->instances.clearDeep();
  delete o;
  
  while(CLOSED.count()>0){
    o=(O_el*) CLOSED.removeTop(&x);
    o->instances.clearDeep();
    delete o;
  }

  return M;
}




List* compress2(List* Gl){
  // two stage compression
  In_MetaGraph *M1;
  List* Ml,*Ml0;
  int x;
  double compress_rate;

  Ml=compress1(Gl);

  // find optimal substructure

  x=0;
  while(1){
    cout << x++ << " . pass > Compressor\n"; 
    optimalSubstructure2(Ml,compress_rate);
    cout << "                     " << compress_rate << " compression rate\n";
    if(compress_rate>1) break;

  }
  return Ml;

}






In_MetaGraph* hierarchy4(In_MetaGraph*& M){
// divide randomly in two parts
  In_MetaGraph* H,*H1,*H2,*M1,*M2,*H3;
  MetaGraph* MG;
  int d,x,k,r,inner;
  int tcon;
  int tsize,tdegree;

  SortedList Best;
  Best.descending();

  if(M->parts.count()==0)
    return M;

  if(M->parts.count()==1){   
    H=(In_MetaGraph*) M->parts.get(0);
    M=hierarchy4(H);
    return M;
  }

  M->parts.reset();
  while(H=(In_MetaGraph*) M->parts.getnext()){
    d=H->degree(inner);
    H=hierarchy3(H);
    if(H->size()<=2)   // if size under 3 then use outer degree to judge
      inner=d;
    Best.insert(H,d,inner);
  }

  double v,tvalue;
  while(Best.count()>1){
    tvalue=Best.TopValue();
    
    r=0;
    H=(In_MetaGraph*) Best.get(0,tsize,v);
    for(x=1;x<Best.count();x++){
      H2=(In_MetaGraph*) Best.get(x,k,v);
      if(v>tvalue) break;
      if(k>tsize){
	H=H2;
	tsize=k;
	r=x;
      }
    }
    
    Best.initNext();
    x=0;
    while(x<=r){ Best.getnext(&k);x++;}
    Best.deleteCurrent();
    

    Best.initNext();
    tsize=1000;
    tdegree=0;
 
    H2=NULL;
    for(x=0;x<Best.count();x++){
      H1=(In_MetaGraph*) Best.get(x);
      if(tcon=H->connectedTo(H1)){
	
	if(tcon>=tdegree){
	  if((tcon==tdegree)&&(tsize<=H1->size())) continue;
	  tsize=H1->size();
	  r=x;
	  tdegree=tcon;
	  H2=H1;
	}
      }
    }
    


    gmt_assert(H2!=NULL);
    Best.initNext();
    x=0;
    while(x<=r){ Best.getnext(&k);x++;}
    Best.deleteCurrent();
		

    H3=new In_MetaGraph;
    MG=new MetaGraph;
    H3->init(MG,H->graph);
    H3->add(H);
    H3->add(H2);

    gmt_assert(H3->tok->length()==H->tok->length()+H2->tok->length());

    d=H3->degree(inner);
    if(H3->size()<3) 
      inner=d;
      
    Best.insert(H3,d,inner);
  }

  
  M=H3;
  return H3;

}
