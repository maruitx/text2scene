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


#include "AdjazenzMatrix.h"

AdjazenzMatrix::AdjazenzMatrix(){
  matrix = 0;
  Dimension = 0;
  edgeDimension=0;
  BothSides=0;
  NodeAttributes=NULL;
  EdgeAttributes=NULL;
  COMPLEX_ATTRIBUTES=0;
  LabelOffset=0;
  Blimit=0;
  Bounds=NULL;
  isGrain=0;
  name=NULL;
  real_Dimension=0;
  temp_Dimension=0;
  real_edgeDimension=0;
  temp_edgeDimension=0;
  IDENTICAL_EDGES_ONLY=0;
  owner=NULL;
  edgesAllocated=0;
  edge_access=0;
  expandable_edges_flag=0;
}

AdjazenzMatrix::~AdjazenzMatrix(){
  clear();
}


int
AdjazenzMatrix::isDirected(){
  return Directed;
}


AdjazenzMatrix::AdjazenzMatrix(AdjazenzMatrix& am){
  clear();
  concatMatrix(&am);

}


AdjazenzMatrix
AdjazenzMatrix::operator=(AdjazenzMatrix& am){

  if(this==&am) return *this;

  clear();
  IDENTICAL_EDGES_ONLY=am.IDENTICAL_EDGES_ONLY;
  concatMatrix(&am);
  return *this;

}



void
AdjazenzMatrix::clear(){
  int x,y;
  EdgeList* n,*m;

  if(matrix) {
    gmt_assert(Dimension);
    for(x=0;x<Dimension;x++){
      for(y=0;y<Dimension;y++){
	n=matrix[x][y].next;
	while(n){
	  m=n->next;
	  delete n;
	  n=m;
	}
      }
      delete matrix[x];
    }
    delete matrix;
  }

  if(Dimension>0) delete[] NodeAttributes;
  if(edgesAllocated>0) delete[] EdgeAttributes;
  Dimension=0;
  edgesAllocated=0;
  edgeDimension=0;
  matrix=NULL;
  NodeAttributes=NULL;
  EdgeAttributes=NULL;
  
 
  deleteSuperNodes();

}


void
AdjazenzMatrix::doOnlyIdenticalEdges(){
  IDENTICAL_EDGES_ONLY=1;
}


void
AdjazenzMatrix::createNow(int directed){
  int x,con;
  AttId at;
  int edg;
  con=0;
  if(matrix==NULL){
    AttId* t;
    EdgeInfo* e;

    if(real_Dimension==0) return;

    t=NodeAttributes;
    e=EdgeAttributes;
    NodeAttributes=NULL;
    EdgeAttributes=NULL;
    

    create(real_Dimension,directed);
    for(x=0;x<real_Dimension;x++)
      setNodeAttributeId(x,t[x]);

    for(x=0;x<real_edgeDimension;x++){
      if(IDENTICAL_EDGES_ONLY){
	if(isEdge(e[x].n1,e[x].n2)){
	  initNext(e[x].n1,e[x].n2);
	  while((at=isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	    if(at==e[x].edgeAtt) continue;
	  }
	}
      }
	
      setEdge(e[x].n1,e[x].n2,con);
      setEdgeAttributeId(con,e[x].edgeAtt,e[x].n1,e[x].n2);
      con++;
    }

    real_Dimension=0;
    temp_Dimension=0;
    real_edgeDimension=0;
    temp_edgeDimension=0;
    
    if(t)
      delete[] t;
    if(e)
      delete[] e;
  }
}

void
AdjazenzMatrix::create(int dimension,int directed){
  int x,y;


//  clear();

  
  Dimension = dimension;
  edgeDimension = 0;
  Directed = directed;
  

  matrix = new EdgeList* [Dimension];
  for(x=0;x<Dimension;x++) matrix[x] = new EdgeList[Dimension];

  for(x=0;x<Dimension;x++)
    for(y=0;y<Dimension;y++){
      matrix[x][y].edge = -1;
      matrix[x][y].next=NULL;
    }

  NodeAttributes = new AttId[Dimension];
#ifdef NEW_ATTOBJECT
  for(x=0;x<Dimension;x++) NodeAttributes[x].set(-1,NULL,0);
#else
  for (x = 0; x < Dimension; x++) NodeAttributes[x] = -1;
#endif

  int numE=Dimension*15;
  if(Dimension<3) numE+=20;
  edgesAllocated=numE;
  EdgeAttributes = new EdgeInfo[edgesAllocated];  /* Estimate  */



///  for(x=0;x<Dimension*Dimension;x++){
 for(x=0;x<numE;x++){
#ifdef NEW_ATTOBJECT
   EdgeAttributes[x].edgeAtt.set(-1,NULL,0);
#else
   EdgeAttributes[x].edgeAtt=-1;
#endif   
   EdgeAttributes[x].n1=0;
   EdgeAttributes[x].n2=0;
 } 
}


void
AdjazenzMatrix::setGrain(int i){
  isGrain=i;
}


void
AdjazenzMatrix::switchAttributeWithLabel(AttributeClass * ATT){
  int x;

  for(x=0;x<Dimension;x++)
#ifdef NEW_ATTOBJECT
    NodeAttributes[x]=ATT->Label(NodeAttributes[x]);
  for(x=0;x<edgeDimension;x++)
    EdgeAttributes[x].edgeAtt=ATT->Label(EdgeAttributes[x].edgeAtt);

#else
  


  printf("Illegal switchAttributeWithLabel use!\n");
  gmt_assert(0);
#endif  
 
}


void
AdjazenzMatrix::randomMerge(int labels,int expect,int varianz,AdjazenzMatrix* subgraph){
  int z,x,y;
  int connect,edg;
  int subdim;

  if(subgraph){

    subdim=Dimension; 
 
    concatMatrix(subgraph);


    edg=edgeDimension-1;
    
#if 0    
    connect=int(rando()*float(varianz)+float(expect));
    if(connect>subdim) connect = subdim;
    y = subdim;
    z=(int) zufall()/MAX * (labels-1);
    if(z<=0) z=1;
    for(x=0;x<connect;x++){
      edg++;
      
      setEdge(x,y,edg);
      setEdgeAttributeId(edg,z,x,y);
#if 0   
      z = (z + 1) % labels;
#else 
      z = (int) zufall()/MAX * (labels-1);
#endif         
      if(z<=0) z=1;


      y++;
      if(y>Dimension) break;
    }
  
#endif  
  }

  connectSingleTrees(labels); 
}




void
AdjazenzMatrix::randomGraph(int dimension,int directed,int labels,int expect,int varianz,AdjazenzMatrix* subgraph,int labeloffset){

  LabelOffset=labeloffset;
  
  randomGraph(dimension,directed,labels,expect,varianz,subgraph);

  LabelOffset=0;
  
  
}


void
AdjazenzMatrix::randomGraph(int dimension,int directed,int labels,int expect,int varianz,AdjazenzMatrix* subgraph){
  
  float connect,rat;
  int nach;
  int x,y,yy,subdim;
  int edg;

  clear();
    
  edg=-1;
  
  if(subgraph){
    subdim = dimension - subgraph->Dimension;
    create(subdim,directed);
  }else{
    create(dimension,directed);
  }

  /** it assumes, that the attributes start with index lllLabelOffset**/

  /** it also assumes, that there are any Attributes avalibale **/
  
  y=rand()%labels;
  yy=y+LabelOffset;
  if(yy==LabelOffset) yy++;
  for(x=0;x<Dimension;x++){
#ifdef NEW_ATTOBJECT
    AttId at;
    at.set(yy,NULL,0);
    setNodeAttributeId(x,at);
#else
    setNodeAttributeId(x,yy);
#endif 
    y = (y + 1) % labels ;
    yy=y + LabelOffset ;
    if(yy==LabelOffset) yy++;
    
  }
 

  y=rand()%labels ;
  yy=y + LabelOffset;
  if(yy==LabelOffset) yy++;
  for(x=0;x<Dimension;x++){
    int loc;
    connect=int(rando()*float(varianz)+float(expect));
    if(connect>=Dimension) connect = Dimension -1;
    
    loc=0;
    
    while((degree(x)<connect)&&(loc<Dimension*3)){
      loc++;
      rat=zufall()/double(MAX);
      nach = (int) rat * Dimension;
      if(x!=nach) {
	
	
	if(degree(nach)<expect+varianz){
	  if(isEdge(x,nach)<0){
	    edg++;
	    setEdge(x,nach,edg);
#ifdef NEW_ATTOBJECT
	    AttId at;
	    at.set(yy,NULL,0);
	    setEdgeAttributeId(edg,at,x,nach);
#else
	    setEdgeAttributeId(edg,yy,x,nach);
#endif
	  }
	  y = (y + 1) % labels;
	  yy=+LabelOffset;
	  if(yy==LabelOffset) yy++;
	  	  
	}
      }
    }
  }


  if(subgraph){
    int z;
    concatMatrix(subgraph);
    edg=edgeDimension-1;
    connect=int(rando()*float(varianz)+float(expect));
    if(connect>subdim) connect = subdim;
    y = subdim;
    z=rand()% labels ;
    yy=z+ LabelOffset;
    if(yy==LabelOffset) yy++;
    for(x=0;x<connect;x++){
      edg++;
      gmt_assert((!(isEdge(x,y)>=0))&&(!isEdge(y,x)>=0));
      setEdge(x,y,edg);
#ifdef NEW_ATTOBJECT
      AttId at;
      at.set(yy,NULL,0);
      setEdgeAttributeId(edg,at,x,y);
#else
      setEdgeAttributeId(edg,yy,x,y);
#endif     
      z = (z + 1) % labels ;
      yy=z+ LabelOffset;
      if(yy==LabelOffset) yy++;
      y++;
      if(y>=Dimension) break;
    }
  }
    
  connectSingleTrees(labels);
}


int
AdjazenzMatrix::connectSingleTrees(int labels){

  int x,y,c,z,start,changes,changes2;
  int *color;
  int edg;

  edg=edgeDimension;
  color = new int[Dimension];
  for(x=0;x<Dimension;x++) color[x]=0;

  c=1;

  changes=1;
  start=0;
  while(changes){
    changes=0;

    x=start;
//    color[start]=c;
    while((x<Dimension)&&(color[x]>0)) x++;
  
    if(x<Dimension) {
      start=x;
      changes=1;
      c++;
      color[start]=c;

      changes2=1;
      while(changes2){
	changes2=0;
	for(x=start;x<Dimension;x++){
	  
	  if(color[x]==c){
	    for(y=0;y<Dimension;y++){
	      if(color[y]==c) continue;
	      if((isEdge(x,y)>=0)||(isEdge(y,x)>=0)){
		changes2=1;
		color[y] = c;
	      }
	    }
	  }
	}
      }
    }
  }

  if(labels==0){
    if(c>2) return 0;
    else return 1;
  }
 
  for(z=Dimension-1;z>=0;z--){
    c=color[z];
    
    x=z;
    while((x>=0)&&(color[x]==c)) x--;
    
    if(x<0) break;

    if(color[x]!=c){
	int c1;
	c1=color[x];
	gmt_assert((!(isEdge(z,x)>=0))&&(!isEdge(x,z)>=0));
	setEdge(z,x,edg);

#ifdef ROTHFILE
	int l=3;
#else
	int l=(int) (zufall()/((double) MAX) * ((double) labels-1));
	int yy=l + LabelOffset;
	if(yy<=LabelOffset) yy=LabelOffset+1;
#endif 
#ifdef NEW_ATTOBJECT
	AttId at;
	at.set(yy,NULL,0);
	setEdgeAttributeId(edg,at,z,x);
#else

	setEdgeAttributeId(edg,yy,z,x);
#endif
	edg++;
	for(y=0;y<Dimension;y++) 
	  if(color[y]==c1) color[y]=c;
      }
    }


  delete color;
  return 1;
}



void
AdjazenzMatrix::setEdge(int x,int y,int edge){
  EdgeList *ptr,*sptr;

  gmt_assert((int) matrix);
  gmt_assert((x<Dimension)&&(y<Dimension));

  gmt_assert(edge<edgesAllocated);

  if(matrix[x][y].edge==-1){
    matrix[x][y].edge=edge;
  }else{

    if(matrix[x][y].edge==edge) {
      if(x==y) return;
      printf("FATAL: identical edges!");
      gmt_assert(0);
    }

    ptr=matrix[x][y].next;
    
    sptr=NULL;
    while(ptr!=NULL){
      if(ptr->edge==edge) {
	if(x==y) return;
	printf("FATAL: identical edges!");
	gmt_assert(0);
      }
      sptr=ptr;
      ptr=ptr->next;
    }

    if(sptr==NULL){
      matrix[x][y].next=new EdgeList;
      ptr=matrix[x][y].next;
    }else{
      sptr->next=new EdgeList;
      ptr=sptr->next;
    }

    

    ptr->edge = edge;
    ptr->next=NULL;

  }

  if(!BothSides) edgeDimension ++;

 
  if((!Directed)&&(x!=y)){ 
    if(!BothSides){
      BothSides=1;
      setEdge(y,x,edge);
      BothSides=0;
    }else{BothSides=0;}
  }
  

}


void 
AdjazenzMatrix::setNodeAttributeId(int x,AttId &att){
  int i;
  if(matrix==NULL){
    if(x>temp_Dimension-1){
      AttId *t;
      t=new AttId[x*2+1];
      if(temp_Dimension>0){  
	for(i=0;i<temp_Dimension;i++)
	  t[i]=NodeAttributes[i];
	
	delete[] NodeAttributes;
      }
      NodeAttributes=t;
      temp_Dimension=x*2+1;
    }
    real_Dimension++;    
  }
  NodeAttributes[x] = att;
}

AttId
AdjazenzMatrix::getNodeAttributeId(int x){
  if(x<Dimension)
    return NodeAttributes[x];
  else{
    x=x-Dimension;
    Tsnode* sn=(Tsnode*) SuperNodes.get(x);
    return sn->at;
  }
}

void 
AdjazenzMatrix::setEdgeAttributeId(int x,AttId &att,int n1,int n2){
  if(matrix==NULL){
    int i;
    if(x>temp_edgeDimension-1){
      EdgeInfo *t;
      t=new EdgeInfo[x*2+1]; 
      if(temp_edgeDimension>0){  
	for(i=0;i<temp_edgeDimension;i++){
	  t[i].n1=EdgeAttributes[i].n1;
	  t[i].n2=EdgeAttributes[i].n2;
	  t[i].edgeAtt=EdgeAttributes[i].edgeAtt;
	}
      
	delete[] EdgeAttributes;
      }
      EdgeAttributes=t;
      temp_edgeDimension=x*2+1; 
    }  
  real_edgeDimension++;
  }

  EdgeAttributes[x].edgeAtt = att;
  EdgeAttributes[x].n1 = n1;
  EdgeAttributes[x].n2 = n2;
}

AttId
AdjazenzMatrix::getEdgeAttributeId(int x,int* n1,int *n2){

#ifdef STUDIES
  edge_access++;
#endif

  if((n1)&&(n2)){
    *n1=EdgeAttributes[x].n1;
    *n2=EdgeAttributes[x].n2;
  }
  
  return EdgeAttributes[x].edgeAtt;
}


int 
AdjazenzMatrix::isEdge(int x,int y){

#ifdef STUDIES
  edge_access++;
#endif


  gmt_assert((int) matrix);

  //assert((x<Dimension)&&(y<Dimension));
  if(!((x<Dimension)&&(y<Dimension))){
      return -1;
  }

  // check for expandable edges
  
  if((matrix[x][y].edge<0)&&(expandable_edges_flag)){
    int ex,ey;
    expandableEdges(x,y,ex,ey);
    return matrix[ex][ey].edge;
  }else{

    return matrix[x][y].edge;
  }
}


int 
AdjazenzMatrix::edgeDegree(int x,int y){
  int n;
  EdgeList *ptr;

  gmt_assert((int) matrix);
  gmt_assert((x<Dimension)&&(y<Dimension));

  if(matrix[x][y].edge==-1) return 0;

  ptr=matrix[x][y].next;
  
  n=1;

  while(ptr!=NULL){n++;ptr=ptr->next;}

  return n;
}



void 
AdjazenzMatrix::initNext(int x){
  NextIndex=0;
  NextArg=x;
}


int
AdjazenzMatrix::getnext(int* out,int* in, AttId* at){
  int x;

#ifdef STUDIES
  edge_access++;
#endif


  for(x=NextIndex;x<edgeDimension;x++)
    if((EdgeAttributes[x].n1==NextArg)||(EdgeAttributes[x].n2==NextArg))
      {
	*out=EdgeAttributes[x].n1;
	*in=EdgeAttributes[x].n2;
	*at=EdgeAttributes[x].edgeAtt;
	NextIndex=x+1;
	return x;
      }

  return -1;
}


void
AdjazenzMatrix::initNext(int x,int y){
  
  //check for expandable edges

  if((x<Dimension)&&(y<Dimension)){
    if((matrix[x][y].edge<0)&&(expandable_edges_flag)){
      int ex,ey;
      expandableEdges(x,y,ex,ey);
      NextPtr=&matrix[ex][ey];
      
    }else{
      
      NextPtr=&matrix[x][y];
    }
    SuperEdge=0;
  }else{
    SuperEdge=1;
    superConnX=0;
    superConnY=0;
    currX=x;
    currY=y;
    NextPtr=NULL;
    nextSuperEdge(currX,currY);
  }
}


void
AdjazenzMatrix::nextSuperEdge(int x,int y){
  int* p,*px,*py;
  int *dirx,*diry;
  int *dir;

  g_snx=NULL;
  g_sny=NULL;

  if(x<Dimension){
    y=y-Dimension;
    g_sny=(Tsnode*) SuperNodes.get(y);
      
    while(superConnY<g_sny->conns.count()){
      p=(int*) g_sny->conns.get(superConnY);
      dir=(int*) g_sny->connsDir.get(superConnY);
      
      if(NextPtr==NULL)
	NextPtr=&matrix[x][*p];  // in case the call came from initNext
      else
	NextPtr=NextPtr->next;
      
      while(NextPtr!=NULL){
	if(NextPtr->edge>-1)
	  if(EdgeAttributes[NextPtr->edge].edgeAtt.values[6]==(*dir))
	    return;
	NextPtr=NextPtr->next;
      }
      superConnY++;
    }
    return; 
  }

/* if only one vertex is super

   _____  _____
               \

	       */



  if(y<Dimension){
    x=x-Dimension;
    g_snx=(Tsnode*) SuperNodes.get(x);
    
    while(superConnX<g_snx->conns.count()){
      p=(int*) g_snx->conns.get(superConnX);
      dir=(int*) g_snx->connsDir.get(superConnX);
      
      if(NextPtr==NULL)
	NextPtr=&matrix[*p][y];  // in case the call came from initNext
      else
	NextPtr=NextPtr->next;
      
      while(NextPtr!=NULL){
	if(NextPtr->edge>-1)
	  if(EdgeAttributes[NextPtr->edge].edgeAtt.values[5]==(*dir))
	    return;
	NextPtr=NextPtr->next;
      }
      superConnX++;
    }
    return;
  }


/*

  if both vertices are super:

   _____   ______ ____
                      \
                       \
                        \
  
  iterate first over Y then over X

			*/


  if((x>=Dimension)&&(y>=Dimension)){
    x=x-Dimension;
    y=y-Dimension;

    g_snx=(Tsnode*) SuperNodes.get(x); 
    g_sny=(Tsnode*) SuperNodes.get(y);
    while(superConnX<g_snx->conns.count()){

      px=(int*) g_snx->conns.get(superConnX);
      dirx=(int*) g_snx->connsDir.get(superConnX);

      if(superConnY<g_sny->conns.count()){
	while(superConnY<g_sny->conns.count()){

	  py=(int*) g_sny->conns.get(superConnY);
	  diry=(int*) g_sny->connsDir.get(superConnY);
      
	  if(NextPtr==NULL){
	    NextPtr=&matrix[*px][*py];  // in case the call came from initNext
	  }else
	    NextPtr=NextPtr->next;
      
	  while(NextPtr!=NULL){
	    if(NextPtr->edge>-1){
	      if((EdgeAttributes[NextPtr->edge].edgeAtt.values[5]==(*dirx))&&
		 (EdgeAttributes[NextPtr->edge].edgeAtt.values[6]==(*diry)))
		return;
	    }
	    NextPtr=NextPtr->next;
	  }
	  superConnY++;
	}
      }
      superConnY=0;
      NextPtr=NULL;
      superConnX++;
    }
    return;
  }
}


int
AdjazenzMatrix::inSuper(int x,int y){

  if((x<Dimension)&&(y<Dimension)) return 0;
  Tsnode* sn,*sn2;
  int* v,*v2;
  if(x<Dimension){  
    y=y-Dimension;
    sn=(Tsnode*) SuperNodes.get(y);
    sn->parts.reset();
    while(v=(int*) sn->parts.getnext()){
      if((*v)==x) return 1;
    }
    return 0;
  }
  if(y<Dimension){
    x=x-Dimension;
    sn=(Tsnode*) SuperNodes.get(x);
    sn->parts.reset();
    while(v=(int*) sn->parts.getnext()){
      if((*v)==y) return 1;
    }
    return 0;
  }
  
  x=x-Dimension;
  y=y-Dimension;
  sn=(Tsnode*) SuperNodes.get(x);
  sn->parts.reset();
  while(v=(int*) sn->parts.getnext()){
    sn2=(Tsnode*) SuperNodes.get(y);
    sn2->parts.reset();
    while(v2=(int*) sn2->parts.getnext()){
      if((*v)==(*v2)) return 1;
    }
  }
  return 0;
  
}


AttId
AdjazenzMatrix::isNextEdgeAttId(int *edg){

  edge_access++;    
  EdgeList* hptr;
  
  if(NextPtr==NULL) return NO_ATTRIBUTE;
  
  if(NextPtr->edge>=0){
    hptr=NextPtr;
    if(edg!=NULL) *edg=NextPtr->edge;
    
    if(SuperEdge==0){
      NextPtr=NextPtr->next;
      return EdgeAttributes[hptr->edge].edgeAtt;
    }else{
      nextSuperEdge(currX,currY);

/* APPLICATION SPECIFIC CODE */
      AttId at;
      at=EdgeAttributes[hptr->edge].edgeAtt;
      if(g_snx==NULL)
	at.values[1]=(NodeAttributes[currX].values[4]-g_sny->at.values[4])/g_sny->at.values[4];
      else if(g_sny==NULL)
	at.values[1]=(g_snx->at.values[4]-NodeAttributes[currY].values[4])/NodeAttributes[currY].values[4];
      else
	at.values[1]=(g_snx->at.values[4]-g_sny->at.values[4])/g_sny->at.values[4];
/* END */
      return at;
    }
    
  }else{
    return NO_ATTRIBUTE;
  }
}



int 
AdjazenzMatrix::numberOfVertices(){

  if(SuperNodes.count()>0)
    return Dimension+SuperNodes.count();
  else
    return Dimension;
}


int 
AdjazenzMatrix::numberOfEdges(){
  return edgeDimension;
}

int
AdjazenzMatrix::degree(int x){
  int i,d;

  gmt_assert((int) matrix);
  gmt_assert(x<Dimension);

  d=0;
#if 0
  for(i=0;i<Dimension;i++)
    if(isEdge(x,i)>=0) d++;

  if(Directed){
      for(i=0;i<Dimension;i++)
	if(isEdge(i,x)>=0) d++;

    }
#else
  int d0;
  for(i=0;i<Dimension;i++)
    if((d0=edgeDegree(x,i))>=0) d+=d0;
  
  if(Directed){
    for(i=0;i<Dimension;i++)
      if((d0=edgeDegree(i,x))>=0) d+=d0;
    
  }
  
#endif
  
  return d;
}


void
AdjazenzMatrix::concatMatrix(AdjazenzMatrix *F){
  int x,y,dim,old_dim,old_edge;
  EdgeList** mymatrix;
  AttId* myatt;
  int edgedim_new;
  EdgeInfo* edgeatt;

  old_dim = Dimension;
  old_edge = edgeDimension;
  dim = Dimension + F->Dimension;
  edgedim_new =edgeDimension + F->edgeDimension;
  
  mymatrix = matrix;
  myatt = NodeAttributes;
  edgeatt = EdgeAttributes;

  Dimension=0;
  edgesAllocated=0;
  edgeDimension=0;
  matrix=NULL;
  NodeAttributes=NULL;   
  EdgeAttributes=NULL;

  create(dim,Directed);

  edgeDimension = edgedim_new;

  if(mymatrix){
    for(x=0;x<old_dim;x++)
      for(y=0;y<old_dim;y++)
	matrix[x][y]=mymatrix[x][y];


    for(x=0;x<old_dim;x++) delete mymatrix[x];
    delete mymatrix;


    for(x=0;x<old_dim;x++)
      NodeAttributes[x]=myatt[x];

    delete[] myatt;

    for(x=0;x<old_edge;x++)
      EdgeAttributes[x]=edgeatt[x];

    delete[] edgeatt;

   }
 
  for(x=old_dim;x<dim;x++)
    for(y=old_dim;y<dim;y++){
      matrix[x][y] = F->matrix[x-old_dim][y-old_dim];
      if(matrix[x][y].edge>=0)
	matrix[x][y].edge+=old_edge;
    }

  for(x=old_edge;x<edgeDimension;x++){
    EdgeAttributes[x]=F->EdgeAttributes[x-old_edge];
    EdgeAttributes[x].n1+=old_dim;
    EdgeAttributes[x].n2+=old_dim;
  }

  for(x=old_dim;x<dim;x++)
    NodeAttributes[x] = F->NodeAttributes[x-old_dim];


}



int
AdjazenzMatrix::readFromFile(AttributeClass* Attr,FILE *file){
/***********************************************/
/*   v number  ( label ( 1  (3  (4  (2  (1.2 )      */
/*   e number  from to  (label  0.2  3.2 )     */
/***********************************************/
  int END;
  char what;
  int stop,fehler,eno,no,no2,Dimension,x;
  AttId att;
  int label;
  int Names[10];
  double Values[10];
  int lim,Modellinit;

  clear();

  fehler=0;
  END=0;
  Modellinit=0;
  isGrain=0;

 if(!feof(file)){
    what=getc_c(file);
    while((!END)&&(!feof(file))&&(!fehler)){
      stop=1;
      switch(what){

      case 'p' : label=getc_c(file);
		 
	         do{
		  
		   label=getc(file);
		 }while(label!=' ');
		 no=(int) get_zahl(file);
		 
		 break;
      case 'v' : no=(int) get_zahl(file);
		 // Nummer in entsprechenden GRaph_knoten fuegen
		 
		
#ifndef ROTHFILE		 
		 what=getc_c(file);
		 x=0;
		 lim=0;
		 
     		 label=(int) get_zahl(file);

		 what = getc_c(file);
		 while((what!=')')&&(what!='\n')&&(!feof(file))){		   
		   
		   if(what=='('){
       		     Values[lim]=get_zahl(file);
		     lim++;
		   }else{
		     fehler=1;
		     break;
		   };
		   what=getc_c(file);
		   x++;
		  };
		
		 if(fehler) break;
		 att=Attr->registerLabel(label,Values,lim);
		 
		 setNodeAttributeId(no,att);
#else
		 Values[0]=0;
		 label=1;
		 lim=0;
		 att=Attr->registerLabel(label,Values,lim);
		 no--;
		 setNodeAttributeId(no,att);

#endif 
		 break;
      case 'e' : eno=(int) get_zahl(file);
		   no=(int) get_zahl(file);
		   no2=(int) get_zahl(file);


#ifndef ROTHFILE

		 setEdge(no,no2,eno);
		 what = getc_c(file);
	
		 label=(int) get_zahl(file);
#else
		 no--;
		 no2--;
		 eno--;

		 setEdge(no,no2,eno);
		 label=2;
#endif
	 
		 lim=0;
		 what=getc_c(file);
		  while((what!=')')&&(what!='\n')&&(!feof(file))){		   
		   
		   if(what=='('){
       		     Values[lim]=get_zahl(file);
		     lim++;
		   }else{
		     fehler=1;
		     break;
		   };
		   what=getc_c(file);
		   x++;
		  };
	

		 if(fehler) break;
		 att=Attr->registerLabel(label,Values,lim);
		 
		 setEdgeAttributeId(eno,att,no,no2);
 
		 break;
		   
      case '%' : label=getc_c(file);
		 if(!Modellinit){
		   int dir;
		   dir=0;
		   while((label!='d')&&(!feof(file))) label=getc_c(file);
		   dir= (int) get_zahl(file);
		   while((label!=':')&&(!feof(file))) label=getc_c(file);
		   Dimension = (int) get_zahl(file);
		   create(Dimension,dir);
		   Modellinit=1;
		 }
		 while((label!='\n')&&(!feof(file))){
		     label=getc_c(file);
		   };
		   break;
      case '@' : label=getc_c(file);
		 isGrain=1;
		 while((label!='\n')&&(!feof(file))){
		     label=getc_c(file);
		   };
		   break;
        case '-' : END=1;Modellinit=0;break; 
	case '\n': break;
       default : fehler=1;break;
       };
		 
    what=getc_c(file);
   };
   if(fehler)
     return 0;
   
#ifdef ROTHFILE
  connectSingleTrees(3);
#endif
  }else{
    return 0;
  }
  return 1;
}


void
AdjazenzMatrix::read(char* fname){
  int x,y,e;
  FILE* file;
  EdgeList* ptr;

  file=fopen(fname,"r");
  gmt_assert(file!=NULL);

  clear();
  
  fread((char*)&Dimension,sizeof(int),1,file);
  fread((char*)&e,sizeof(int),1,file);
  fread((char*)&Directed,sizeof(int),1,file);
  fread((char*)&number,sizeof(int),1,file);

  create(Dimension,Directed);
  
  char text[256];
  fread((char*)text,sizeof(char),256,file);
  if(strlen(text)>0){
    name=new char[strlen(text)+1];
    strcpy(name,text);
  }

#ifdef NEW_ATTOBJECT
  for(x=0;x<Dimension;x++)
    NodeAttributes[x].read(file);
  
  fread((char*)EdgeAttributes,sizeof(EdgeInfo),e,file);
  for(x=0;x<e;x++)
    EdgeAttributes[x].edgeAtt.read(file);
  
#else
 
  fread((char*)NodeAttributes,sizeof(AttId),Dimension,file);
  fread((char*)EdgeAttributes,sizeof(EdgeInfo),e,file);
#endif
  
  for(x=0;x<e;x++)
    setEdge(EdgeAttributes[x].n1,EdgeAttributes[x].n2,x);
  
  fclose(file);
}

void
AdjazenzMatrix::write(char* fname){
  int x,y;
  FILE* file;
  EdgeList* ptr;

  file=fopen(fname,"w");
  gmt_assert(file!=NULL);

  fwrite((char*)&Dimension,sizeof(int),1,file);
  fwrite((char*)&edgeDimension,sizeof(int),1,file);
  fwrite((char*)&Directed,sizeof(int),1,file);
  fwrite((char*)&number,sizeof(int),1,file);
  

  char text[256];
  if(name!=NULL)
    strcpy(text,name);
  else
    text[0]='\0';
  fwrite((char*)text,sizeof(char),256,file);

#ifdef NEW_ATTOBJECT
  for(x=0;x<Dimension;x++)
    NodeAttributes[x].write(file);
  fwrite((char*)EdgeAttributes,sizeof(EdgeInfo),edgeDimension,file);
  for(x=0;x<edgeDimension;x++){
    EdgeAttributes[x].edgeAtt.write(file);
  }
  
#else
  fwrite((char*)NodeAttributes,sizeof(AttId),Dimension,file);
  fwrite((char*)EdgeAttributes,sizeof(EdgeInfo),edgeDimension,file);
#endif
  fclose(file);

}


int
AdjazenzMatrix::readFromFile(AttributeClass* Attr,char* name){
  FILE* file;
  int r;

  file=fopen(name,"r");
#if 0
  assert(file!=NULL);
#else
  if(!file) return 0;
#endif
  r=readFromFile(Attr,file);
  fclose(file);
  return r;

}



int
AdjazenzMatrix::writeToFile(char* gname,int number,AttributeClass* Attr,FILE *file){
  int x,i,y;
  char out[1024];
  char st[256];
  double *val;
  int n,z;

#ifdef NEW_ATTOBJECT

  strcpy(out,"% d 1 Knoten:");
  sprintf(st,"%d",Dimension);
  strcat(out,st);
  strcat(out,"   Labels:");
  sprintf(st,"%d",2);
  strcat(out,st);
  strcat(out,"   Kanten:");
  sprintf(st,"%d",edgeDimension);
  strcat(out,st);
  strcat(out,"\n");
  fputs(out,file);

  strcpy(out,"%DEFNAME:");
  if(Attr){
    strcat(out,Attr->name());
  }else{
    strcat(out,gname);
    strcat(out,"_Def");
  }
    
  strcat(out,"\n");
  fputs(out,file);

  if(isGrain){
    strcpy(out,"@\n");
    fputs(out,file);
  }

  
#if 0
    out="% Connectivity:"+to_string(ratioy)+"/"+to_string(ratiox)+"\n";
    (*dateix).put(out);
#endif

  strcpy(out,"p ");
  strcat(out,gname);
  sprintf(st,"%d",number);
  strcat(out," ");
  strcat(out,st);
  strcat(out,"\n");
  fputs(out,file);
  
  for(x=0;x<Dimension;x++){
       
    strcpy(out,"v ");
    sprintf(st,"%d",x);
    strcat(out,st);
    
    if(COMPLEX_ATTRIBUTES){
   
      val=Attr->valueArray(NodeAttributes[x],&n);
      strcat(out," (1 ");
      for(z=0;z<n;z++){
	strcat(out," (");
	sprintf(st,"%f",val[z]);
	strcat(out,st);
      }
      strcat(out," )\n");
    }else{
      strcat(out," (");
      sprintf(st,"%d",NodeAttributes[x].label());
      strcat(out,st);
      strcat(out," )\n");
    }

    fputs(out,file);
  };
  
  i=0;
  for(x=0;x<Dimension;x++){
    for(y=0;y<Dimension;y++){
      int e;
      if((e=isEdge(x,y))>=0){
	strcpy(out,"e  ");
	sprintf(st,"%d",i);
	strcat(out,st);
	strcat(out,"  ");
	sprintf(st,"%d",x);
	strcat(out,st);
	strcat(out," ");	
	sprintf(st,"%d",y);
	strcat(out,st);

	if(COMPLEX_ATTRIBUTES){

	  val=Attr->valueArray(EdgeAttributes[e].edgeAtt,&n);
	  strcat(out," (2 ");
	  for(z=0;z<n;z++){
	    strcat(out," (");
	    sprintf(st,"%f",val[z]);
	    strcat(out,st);
	  }
	  strcat(out," )\n");

	}else{
	  strcat(out," (");
	  sprintf(st,"%d",EdgeAttributes[e].edgeAtt.label());
	  strcat(out,st);
	  strcat(out," )\n");
	}

  	
	fputs(out,file);
	i++;
      }
    }
  }
#endif
  return 1;
}




void 
AdjazenzMatrix::setName(char* n){
  if(n==NULL) return;
  if(name==NULL)
    name=new char[strlen(n)+1];
  
  strcpy(name,n);
  
}


char* 
AdjazenzMatrix::Name(){
  return name;
}


void 
AdjazenzMatrix::setNumber(int n){
  number=n;
}
  

int 
AdjazenzMatrix::Number(){

  return number;
}




void setLabelstring(int x,double t,double w, double i,char* out){
  char st[256];

  strcpy(out,"l ");
  sprintf(st,"%d",x);
  strcat(out,st);
  strcat(out," ' d 0 t "); 
//  sprintf(st,"%f",t);
//  strcat(out,st);
  strcat(out," w ");
//  sprintf(st,"%f",w);
//  strcat(out,st);
  strcat(out," i ");
  sprintf(st,"%f",i);
  strcat(out,st);
  strcat(out," c ");
  sprintf(st,"%f",i);
  strcat(out,st);
 strcat(out,"\n");
}

void setTablestring(int x,int y, double sub,double var,char* out){
  char st[256];
  double r;


  sprintf(st,"%d",x);
  strcpy(out,st);
  strcat(out," , ");
  sprintf(st,"%d",y);
  strcat(out,st);
  strcat(out," : + ");

#ifdef NORMALDISTANCE
#ifdef NORMALERROR

  r=rando()*var+sub;
#else

  r=((double)(rand() % 1000))/1000.0 * fabs(var-sub) + sub;

#endif
#else

  r=abs(x-y) + sub;

#endif
  sprintf(st,"%f",r);
  strcat(out,st);
  strcat(out,"\n");

}





void
AdjazenzMatrix::createDef(char* name, int labels,double ins,double var,double sub,int* bounds,int blimit){

  Bounds=bounds;
  Blimit=blimit;
  
  createDef(name, labels, ins, var, sub);

  Bounds=NULL;
  Blimit=0;
}


void
AdjazenzMatrix::createDef(char* name, int labels,double ins,double var,double sub){
  FILE* file;
  char out[1024];
  char st[256];
  char l[8],t[8];
  int x,y,z;

  file=fopen(name,"w");

  strcpy(l,"=L=\n");
  strcpy(t,"=T=\n");
  
  fputs("#\n",file);

  for(x=1;x<=labels;x++){
    fputs(l,file);
    
    setLabelstring(x,0.1,2,ins,out);
    fputs(out,file);
  }

  if(Blimit>0){
    for(x=0;x<Blimit;x++){
      int g,z;
      if(x==0) g=1;
      else g=(x)*Bounds[x-1]+1;
      for(y=g;y<Bounds[x]+g;y++){
	for(z=y+1;z<Bounds[x]+g;z++){
	  fputs(t,file);
	  setTablestring(y,z,sub,var,out);
	  fputs(out,file);
	}
      }
    }
  }else{
    for(x=1;x<=labels;x++){
      for(y=x+1;y<=labels;y++){
	fputs(t,file);
	setTablestring(x,y,sub,var,out);
	fputs(out,file);
      }
    }
  }
  fputs("=E=",file);
  fclose(file);
}




Token*
AdjazenzMatrix::orderCoherent(){
  int* H,*Way;
  int x,y,i,k;
  int good,out,in;
  AttId at;
  Token *Ret;

  i=0;
  H=new int[Dimension];
  Way=new int[Dimension];

  for(x=0;x<Dimension;x++) H[x]=0;

  /* Start */

  Way[i]=0;
  H[0]=1;

  for(i=1;i<Dimension;i++){

    good=0;
    for(k=0;k<i;k++){
      
      initNext(Way[k]);
      
      while(getnext(&out,&in,&at)>-1){
	if(out==Way[k]){
	  if(H[in]==0) {
	    Way[i]=in;
	    H[in]=1;
	    good=1;
	    break;
	  }
	}else{
	  if(H[out]==0){
	    Way[i]=out;
	    H[out]=1;
	    good=1;
	    break;
	  }
	}
      }
      if(good) break;
    }
    if(!good){
      printf("Graph is not one component!\n");
      for(x=0;x<Dimension;x++)
	if(H[x]==0) {
	  Way[i]=x;
	  H[x]=1;
	  break;
	}
    }
  }
  
  Ret=new Token(Dimension);

  for(x=0;x<Dimension;x++) Ret->set(x,Way[x]);
  delete H;
  delete Way;
  return Ret;

}


double
AdjazenzMatrix::transformGraph(int eps,double* types,AttributeClass *ATT,int startn,int endn){
  int x,y;
  int t;
  double err,serr,ferr;
  int* unodes;
  int* uedges;
  EdgeList* tedge,*medge,*sedge;
  int lat;
  AttId newlabel;
  int out,in,i;
  AttId at;
  int deletedNodes,deletedEdges;
  AttId* nodeatt;
  EdgeInfo* edgeatt;
  int oldDimension,oldedgeDimension;
  EdgeList** mymatrix;
  int edg;

#ifndef NEW_ATTOBJECT
  std::printf("Illegal transformGraph use!\n");
  gmt_assert(0);
#else

  if(Dimension<3) return 0;

  edg=-1;
  err=0;
  unodes=new int[Dimension];
  uedges=new int[edgeDimension];

  deletedNodes=0;
  deletedEdges=0;

  for(x=0;x<Dimension;x++) unodes[x]=0;
  for(x=0;x<edgeDimension;x++) uedges[x]=0;


/*  Node substitution   */
  for(x=0;x<types[0];x++){

    do{
      do{
	t=rand() % Dimension;

      }while((unodes[t]>0)||(t<startn)||(t>endn));
      
      at=getNodeAttributeId(t);
      //    lat=AT->Label(at);
      newlabel=ATT->randomLabel(at,eps-err);
      ferr=ATT->labelToLabelError(at.label(),newlabel.label());
    }while(ferr<0);
    
    NodeAttributes[t]=newlabel;
    err=err+ferr;
    unodes[t]=1;
  }
  
  if(err>eps) {
    delete unodes;
    delete uedges;
    return err;
  }

/* edge substitution */
    
  for(x=0;x<types[1];x++){
    
    do{
      do{
	t=rand() % edgeDimension;
	if((unodes[EdgeAttributes[t].n1]==2)||(unodes[EdgeAttributes[t].n2]==2)) uedges[t]=2;
	
      }while((uedges[t]>0)||(EdgeAttributes[t].n1<startn)||(EdgeAttributes[t].n1>endn));
      
      at=EdgeAttributes[t].edgeAtt;
//      lat=ATT->Label(at);

      newlabel=ATT->randomLabel(at,eps-err);
      ferr=ATT->labelToLabelError(at.label(),newlabel.label());
      
      }while(ferr<0);
    EdgeAttributes[t].edgeAtt=newlabel;
    
    err=err+ferr;
    uedges[t]=1;
  }
  


/*  Node deletion */

  
  if(err>eps) {
    delete unodes;
    delete uedges;
    return err;
  }

  for(x=0;x<types[2];x++){

    do{
      t=rand() % Dimension;
    }while((unodes[t]>0)||(t<startn)||(t>endn));;

    at=getNodeAttributeId(t);
    
    ferr=ATT->insertionCost(at);

    initNext(t);

    while(getnext(&out,&in,&at)>-1){
      ferr+=ATT->insertionCost(at);
      if(!((unodes[out]==2)||(unodes[in]==2))) deletedEdges++;

    }

    unodes[t]=2;
    deletedNodes++;
    err=err+ferr;

  }


  if(err<eps){



    
/* edge deletion  */

    


    for(x=0;x<types[3];x++){
      
      do{
	t=rand() % edgeDimension;
	if((unodes[EdgeAttributes[t].n1]==2)||(unodes[EdgeAttributes[t].n2]==2)) uedges[t]=2;
	
      }while((uedges[t]>0)||(EdgeAttributes[t].n1<startn)||(EdgeAttributes[t].n1>endn));
      
      at=EdgeAttributes[t].edgeAtt;
      
      ferr=ATT->insertionCost(at);
      
      err=err+ferr;
      uedges[t]=2;
      deletedEdges++;
    }

  }

  edgeatt=EdgeAttributes;
  nodeatt=NodeAttributes;
  mymatrix=matrix;

  oldDimension=Dimension;
  oldedgeDimension=edgeDimension;

  create(Dimension-deletedNodes,Directed);

  i=0;
  for(x=0;x<oldDimension;x++){
    if(unodes[x]!=2){
      NodeAttributes[i]=nodeatt[x];
      unodes[x]=-i;
      i++;
    }
  }

#if 0
  for(x=0;x<oldedgeDimension;x++){
    if(uedges[x]!=2){
      EdgeAttributes[i].n1=-unodes[edgeatt[x].n1];
      EdgeAttributes[i].n2=-unodes[edgeatt[x].n2];
      EdgeAttributes[i].edgeAtt=edgeatt[x].edgeAtt;      
      uedges[x]=-i;
      i++;
    }
  }
#endif


  for(x=0;x<oldDimension;x++){
    if(unodes[x]!=2){
      for(y=0;y<oldDimension;y++){
	if(unodes[y]!=2){

#if 0	  
	  sedge=NULL;
	  medge=&matrix[x][y];
	  tedge=&mymatrix[x][y];
	  if(tedge->edge>-1){

	    if(uedges[tedge->edge]!=2){
	      medge->edge=-uedges[tedge->edge];
	      medge->next=new EdgeList;
	      sedge=medge;
	      medge=medge->next;
	      medge->next=NULL;
	    }
	    while(tedge->next){
	      tedge=tedge->next;
	      if(uedges[tedge->edge]!=2){
		medge->edge=-uedges[tedge->edge];
		medge->next=new EdgeList;
		sedge=medge;
		medge=medge->next;
		medge->next=NULL;
	      }
	    }
	    
	    if(sedge) {
	      delete sedge->next;
	      sedge->next=NULL;
	    }
	  }
#else
	  if(mymatrix[x][y].edge>-1){
	    int et=mymatrix[x][y].edge;
	    if(uedges[et]!=2){
	      edg++;
	      setEdge(-unodes[x],-unodes[y],edg);
	      setEdgeAttributeId(edg,edgeatt[et].edgeAtt,-unodes[x],-unodes[y]);
	    }
	  }
#endif
	}
      }
    }
  }

  for(x=0;x<oldDimension;x++){
      for(y=0;y<oldDimension;y++){
	EdgeList* n=mymatrix[x][y].next;
	while(n){
	  EdgeList* m=n->next;
	  delete n;
	  n=m;
	}
      }
      delete mymatrix[x];
    }

  
  if(oldDimension)
    delete mymatrix;
  
  if(edgeatt)
    delete[] edgeatt;
  if(nodeatt)
    delete[] nodeatt;
  if(unodes)
    delete unodes;
  if(uedges)
    delete  uedges;


  return err;
#endif
}


void
AdjazenzMatrix::expandableEdges(int x,int y,int& ex,int &ey){
  List lx,ly;
  int *vx,*vy;

  ex=x;
  ey=y;
  
  vx=new int;
  vy=new int;
  *vx=x;
  *vy=y;
  lx.insert(vx);
  ly.insert(vy);
  expand(x,-1,lx); 
  expand(y,-1,ly);
  lx.reset();
  while(vx=(int*) lx.getnext()){
    ly.reset();
    while(vy=(int*) ly.getnext()){
      if(matrix[*vx][*vy].edge>0){
	ex=*vx;
	ey=*vy;
	return;
      }
    }
  }
  lx.clearDeep();
  ly.clearDeep();
}

void
AdjazenzMatrix::expand(int x, int dir,List& lx){
  int i,*vx,e;

// circles are not check for -> infinite loop may result!

  for(i=0;i<Dimension;i++){
    if(x==i) continue;
    if(i==dir) continue;
    if(matrix[x][i].edge>=0){
      e=matrix[x][i].edge;
      if(fabs(EdgeAttributes[e].edgeAtt.values[0]-PI)<0.01){
	vx=new int;
	*vx=i;
	lx.insert(vx);
	expand(i,x,lx);
      }
    }
  }
}



void
AdjazenzMatrix::setSuperNode(List& parts, List &conns, List& connsDir, AttId &at, double err){
  int x;
  Tsnode *sn;

  x=SuperNodes.count();
  
  sn = new Tsnode;
  
  parts.mv(sn->parts);
  conns.mv(sn->conns);
  connsDir.mv(sn->connsDir);
  sn->at=at;
  sn->err=err;
  SuperNodes.insert(sn);
} 




int
AdjazenzMatrix::numberOfSuperNodes(){

  return SuperNodes.count();
}

void
AdjazenzMatrix::deleteSuperNodes(){

  while(SuperNodes.count()>0){
    Tsnode* sn=(Tsnode*) SuperNodes.remove(0);
    sn->conns.clearDeep();
    sn->connsDir.clearDeep();
    sn->parts.clearDeep();
    delete sn;
  }
}
