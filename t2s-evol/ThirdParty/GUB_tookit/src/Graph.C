#include "Graph.h"
#include "SpecialGraph.h"


#ifdef APPLICATION_SPECIFIC
extern complexDump(void* data,int i);
#endif

//AttributeClass Graph::ATT_object=;
int Graph::loadedDefinition=0;
int Graph::basic_steps=0;

//AttributeClass ATT_object;

void attribute_definition(char* name){
//  Graph::ATT_object.loadDefinition(name);
  Graph::loadedDefinition=1;
}


Graph::Graph(){
  Done=0;
  
//   if(!loadedDefinition){
//     char* nam=getenv("NM_DEFINITION_FILE");
//     if(nam){
//       if(!ATT_object.defined()){
// 	//ATT_object.init();
// 	ATT_object.loadDefinition(nam);
//       }
//       loadedDefinition=1;
//     }
//   }
  xVertices=0;
  xEdges=0;
  MaximumVertex=0;
  name[0]='\0';

  AM.expandable_edges_flag=0;
  AM.owner=this;
}


Graph::~Graph(){
  discard();
}

Graph* 
Graph::copy(){
  List lvertices;
  int x,*v;
  
  for(x=0;x<Vertices.count();x++){
    v=new int;
    *v=x;
    lvertices.insert(v);
  }

  Graph* G=getSubgraph(lvertices);
  
  return G;
}


void
Graph::discard(){
  int *ix,k;
  Done=0;
 
  while(Vertices.count()>0){
    ix=(int*) Vertices.removeTop(&k);
    delete ix;
  }
  while(Edges.count()>0){
    ix=(int*) Edges.removeTop(&k);
    delete ix;
  }

  AM.clear();
  MaximumVertex=0;
  xVertices=0;
  xEdges=0;
  VISIBILITY.clear();
}

void
Graph::setName(const char* n){
  strcpy(name,n);
}

char* 
Graph::Name(){
  return name;
}


int
Graph::read(char* gname){
  int x,true_x;
  int *ix,n;
  FILE* file;
  char tname[256];
  discard();

//  if(!loadedDefinition){
//    char* nam=getenv("NM_DEFINITION_FILE");
//    if(nam){
//      ATT_object.loadDefinition(nam);
//      loadedDefinition=1;
//    }
//  }


  AM.read(gname);
  strcpy(tname,gname);
  strcat(tname,".graph");
  file=fopen(tname,"r");
  fread((char*)&n,sizeof(int),1,file);
  xVertices=0;
  for(x=0;x<n;x++){
    xVertices++;
    ix=new int;
    fread((char*)ix,sizeof(int),1,file);
    fread((char*)&true_x,sizeof(int),1,file);
    Vertices.insert(ix,*ix,(double) true_x);
  }
 
  xEdges=0;
  fread((char*)&n,sizeof(int),1,file);
  for(x=0;x<n;x++){
    xEdges++;
    ix=new int;
    fread((char*)ix,sizeof(int),1,file);
    fread((char*)&true_x,sizeof(int),1,file);
    Edges.insert(ix,*ix,(double) true_x);
  }
  fread((char*)name,sizeof(char),256,file);
  fclose(file);

  return 1;
  
}


int
Graph::write(char* gname){
  int true_x,x;
  char tname[256];
  FILE* file;
  AM.write(gname);
  strcpy(tname,gname);
  strcat(tname,".graph");
  file=fopen(tname,"w");
  fwrite((char*)&xVertices,sizeof(int),1,file);
  for(x=0;x<xVertices;x++){
    true_x=(int) Vertices.getKey(x);
    fwrite((char*)&x,sizeof(int),1,file);
    fwrite((char*)&true_x,sizeof(int),1,file);
  }
 
  fwrite((char*)&xEdges,sizeof(int),1,file);
  for(x=0;x<xEdges;x++){
    true_x=(int) Edges.getKey(x);
    fwrite((char*)&x,sizeof(int),1,file);
    fwrite((char*)&true_x,sizeof(int),1,file);
    
  }
  fwrite((char*)name,sizeof(char),256,file);
  fclose(file);

  return 1;
}



void
Graph::done(int directed){
  Done=1;
  AM.createNow(directed);
}


void
Graph::doOnlyIdenticalEdges(){
  
  AM.doOnlyIdenticalEdges();

}


void
Graph::set(int x, int label, double* values, int n){
  int *ix;

  if(x>MaximumVertex) MaximumVertex=x;
  ix=new int;
  *ix=xVertices;
  Vertices.insert(ix,*ix,x);
  AttId at=ATT_object.registerLabel(label,values,n);
  AM.setNodeAttributeId(xVertices,at);
  xVertices++;
}
  

void
Graph::set(int x, int label, double* values, int n, void* complex){
  int *ix;

  if(x>MaximumVertex) MaximumVertex=x;
  ix=new int;
  *ix=xVertices;
  Vertices.insert(ix,*ix,x);
  AttId at=ATT_object.registerLabel(label,values,n);
  at.setComplex(complex,label);
  AM.setNodeAttributeId(xVertices,at);
  xVertices++;
}

void
Graph::resetAttributes(int x, int label, double* values, int n){
  int *ix;
  
  ix=(int*) Vertices.getValue(x);
  gmt_assert(*ix>-1);
  AttId at=ATT_object.registerLabel(label,values,n);
  AM.setNodeAttributeId(*ix,at);
 
}


void 
Graph::resetEdgeAttributes(int edg,int label, double* values,int n){
  int *ie,n1,n2;
  AttId at2;
  ie=(int*) Edges.getValue(edg);
  gmt_assert(*ie>-1);
  AttId at=ATT_object.registerLabel(label,values,n);

  at2=AM.getEdgeAttributeId(*ie,&n1,&n2);
  AM.setEdgeAttributeId(*ie,at,n1,n2);

}


void 
Graph::setEdge(int x, int y, int e, int label, double* values,int n){
  int *ie;
  int *ix,*iy;
  
  ie=new int;
  *ie=xEdges;
  if(e==-1) e=xEdges;
  Edges.insert(ie,*ie,e);
  ix=(int*) Vertices.getValue(x);
  iy=(int*) Vertices.getValue(y);
  
  AttId at=ATT_object.registerLabel(label,values,n);
 
  if(Done) AM.setEdge(*ix,*iy,xEdges);
  AM.setEdgeAttributeId(xEdges,at,*ix,*iy);
  xEdges++;
}


void 
Graph::get(int x, int& label, double *&values, int &n){
  int *ix;
  
  if(x>-1){
    ix=(int*) Vertices.getValue(x);
    
    if(ix==NULL) {
      label=-1;
      values=NULL;
      return;
    }
    
    values=AM.NodeAttributes[*ix].values;
    n=AM.NodeAttributes[*ix].n;
    label=AM.NodeAttributes[*ix].Label;
  }else{
    x=-(x+10);
    gmt_assert(AM.SuperNodes.count()>x);
    getSuperId(x,label,values,n);
    return;
  }
}


void*
Graph::getComplex(int v,int &desc){
  int *ix;
  
  ix=(int*) Vertices.getValue(v);
  
  if(ix==NULL) {
    return NULL;
  }

  if((*ix)<AM.Dimension){

    if(AM.NodeAttributes[*ix].complex_value==NULL)
      return NULL;

    desc=AM.NodeAttributes[*ix].complex_value->descriptor;
    
    return AM.NodeAttributes[*ix].complex_value->data;

  }else{
    printf("WARNING! Error in getIcomplex....cont'd\n");
    // does not apply here!!
  }
  return NULL;
}


int
Graph::numberOfVertices(){
  return AM.numberOfVertices();
}


int
Graph::numberOfEdges(){
  return AM.numberOfEdges();
}


int
Graph::getNumberOfEdges(int v1,int v2){
  int *ix,*iy;
  ix=(int*) Vertices.getValue(v1);
  iy=(int*) Vertices.getValue(v2);

  return AM.edgeDegree(*ix,*iy);
}


int
Graph::getSuperId(int x,int& label, double*&values,int&n){
  Tsnode* sn;
  if(AM.SuperNodes.count()<=x) return 0;
  
  sn=(Tsnode*) AM.SuperNodes.get(x);

  values=sn->at.values;
  n=sn->at.n;
  label=sn->at.Label;
  return 1;

}

int
Graph::getI(int index, int& label, double *&values, int &n){
  int true_x;
  int d=AM.Dimension;
  
  if(AM.Dimension==0)
    d=Vertices.count();

  if(index<d){
    true_x=(int) Vertices.getKey(index);
    label=AM.NodeAttributes[index].Label;
    values=AM.NodeAttributes[index].values;
    n=AM.NodeAttributes[index].n;
    return true_x;
  }else{
    true_x=index-d;
    getSuperId(true_x,label,values,n);
    true_x=-true_x-10;
    return true_x;
  }
}


int
Graph::getI(int index){
  int true_x;
  int d=AM.Dimension;
  
  if(AM.Dimension==0)
    d=Vertices.count();

  if(index<d){
    true_x=(int) Vertices.getKey(index);
  }else{
    true_x=index-d;
    true_x=-true_x-10;
  }
  return true_x;
}


void*
Graph::getIcomplex(int index,int &desc){
  int true_x;

  if(index<AM.Dimension){
    true_x=(int) Vertices.getKey(index);
    if(AM.NodeAttributes[index].complex_value==NULL)
      return NULL;

    desc=AM.NodeAttributes[index].complex_value->descriptor;
    
    return AM.NodeAttributes[index].complex_value->data;

  }else{
    printf("WARNING! Error in getIcomplex....cont'd\n");
    // does not apply here!!
  }
  return NULL;
}


int
Graph::getIndex(int v){
  int *ix;
  if(v>-1){
    ix=(int*) Vertices.getValue(v);
  
    return *ix;
  }else{
    return -(v+10)+AM.Dimension;
  }
}




int 
Graph::getEdgeNR(int x,int y){
  int n; 

  basic_steps++;
  if((x>AM.Dimension)||(y>AM.Dimension)) return 0;
  n=AM.edgeDegree(x,y);
  return n;

}

int
Graph::getEdgeNR(int x){
  int *ind;

  basic_steps++;
  if(x<0) return 0;
  ind=(int*) Vertices.getValue(x);
  
  return AM.degree(*ind);
}

int 
Graph::getEdgesAdjacentTo(int x,int ind ,int &to_vertex,int& label, double* &values, int &n){
  int dir;
  
  return getEdgesAdjacentTo(x,ind,to_vertex,label,values,n,dir);
}


int
Graph::getEdgesAdjacentTo(int x,int ind ,int &to_vertex,int& label, double* &values, int &n,int &dir){
  int *ind_x,i,n1,n2,edg,edge;
  AttId at;
  ind_x=(int*) Vertices.getValue(x);
  AM.initNext(*ind_x);
  i=0;

  basic_steps++;
#define ALL_EDGES
#ifdef ALL_EDGES
  while((edg=AM.getnext(&n1,&n2,&at))!=-1){
    if(i==ind) break;
    i++;
  }
   
  if(n1==(*ind_x)){
    dir=1;
    to_vertex= (int) Vertices.getKey(n2);
  }

  if(n2==(*ind_x)){
    dir=-1;
    to_vertex= (int) Vertices.getKey(n1);
  }
#else
 while((edg=AM.getnext(&n1,&n2,&at))!=-1){
   if(n2==(*ind_x)) continue;
   if(i==ind) break;
   i++;
  }
   
  if(n1==(*ind_x))
    to_vertex= Vertices.getKey(n2);



#endif

  

  edge=(int) Edges.getKey(edg);
  values=AM.EdgeAttributes[edg].edgeAtt.values;
  n=AM.EdgeAttributes[edg].edgeAtt.n;
  label=AM.EdgeAttributes[edg].edgeAtt.Label;
  return edge;
 
}


void
Graph::getEdge(int edge, int &n1,int &n2,int& label, double*& values, int &n){
  int *ind,index;

  ind=(int*) Edges.getValue(edge);
  index=*ind;

  basic_steps++;
  n1=AM.EdgeAttributes[index].n1;
  
  n2=AM.EdgeAttributes[index].n2;
  n1= (int) Vertices.getKey(n1);
  n2= (int) Vertices.getKey(n2);

  values=AM.EdgeAttributes[index].edgeAtt.values;
  n=AM.EdgeAttributes[index].edgeAtt.n;
  label=AM.EdgeAttributes[index].edgeAtt.Label;
  
}


int
Graph::getEdgeId(int index, int &n1,int &n2,int& label, double*& values, int &n){
  
    basic_steps++;
  n1=AM.EdgeAttributes[index].n1;
  n2=AM.EdgeAttributes[index].n2;
  values=AM.EdgeAttributes[index].edgeAtt.values;
  n=AM.EdgeAttributes[index].edgeAtt.n;
  label=AM.EdgeAttributes[index].edgeAtt.Label;
  return (int) Edges.getKey(index);
}


int
Graph::getEdgesBetween(int x,int y, int index,int& label, double*& values, int &n){
  AttId at;
  int edge;
  int i,ind;

  basic_steps++;
  if(AM.isEdge(x,y)<0) return -1;

  AM.initNext(x,y);
  i=0;
  while((at=AM.isNextEdgeAttId(&edge))!=NO_ATTRIBUTE){
    
    if(i==index) break;
    i++;
  }
  ind=(int) Edges.getKey(edge);
  if(i!=index) return -1;
  label=at.label();
  /* Dont use the local variable at because its pointer values will be destroyed */
  values=AM.EdgeAttributes[edge].edgeAtt.values;
  n=at.length();

  return ind;
}



int
Graph::maximumVertex(){
  return MaximumVertex;
}


void
Graph::defaultKeys(){

  int x,y,*ix,*iy,i;
  x=AM.numberOfVertices();
  y=AM.numberOfEdges();

  for(i=0;i<x;i++){
    ix=new int;
    *ix=i;
    Vertices.insert(ix,i,i);
  }

  for(i=0;i<y;i++){
    iy=new int;
    *iy=i;
    Edges.insert(iy,i,i);
  }
  xVertices=x;
  xEdges=y;

}



void
Graph::dump(){
  int n,x,y,label,v,n1,n2;
  double *values;
  int *p;
  Tsnode* sn;

  cout << "Name: " << name << "\n";

  cout << "Vertices: " <<  xVertices << "  Edges: " << xEdges << "\n\n";
  for(x=0;x<xVertices;x++){
    v=getI(x,label,values,n);
    cout << "v: " << x << " L:" << label ;
    for(y=0;y<n;y++)
      cout << " , " << values[y];
    
#ifdef APPLICATION_SPECIFIC
    void *data;
    int desc;
    data=getIcomplex(x,desc);
    complexDump(data,desc);
#endif

    cout << "\n";
  }

  for(x=0;x<xEdges;x++){
    v=getEdgeId(x,n1,n2,label,values,n);
    cout << "e: " << v << " from: " << n1 << " to: " << n2 << " L: " << label ;
    for(y=0;y<n;y++)
      cout << " , " << values[y];
    cout << "\n";
  }
    
  if(AM.SuperNodes.count()>0)
    cout << "SuperNodes: " << AM.SuperNodes.count() << "\n";

  for(x=0;x<AM.SuperNodes.count();x++){
    sn=(Tsnode*) AM.SuperNodes.get(x);
    sn->parts.reset();
    cout << "\n" << x << ". Error:" << sn->err << "  parts: ";
    while(p=(int*) sn->parts.getnext()){
      cout << *p << " ; ";
    }

    cout << "  conns: ";
    sn->conns.reset();
    while(p=(int*) sn->conns.getnext()){
      cout << *p << " ; ";
    }
    getSuperId(x,label,values,n);
    for(y=0;y<n;y++)
      cout << " , " << values[y];
  }
}

void
Graph::setVisibility(int v, int h){
  int k;

  k=VISIBILITY.getValueKey(v);
  if(h==0){    
    if(k==-1){
      VISIBILITY.insert(this,v,v);
      if(v<-1){
	int j,i,*v1;
	i=-(v+10);
	Tsnode* sn=(Tsnode*) AM.SuperNodes.get(i);
	sn->parts.reset();
	while(v1=(int*) sn->parts.getnext()){
	  j=getI(*v1);
	  setVisibility(j,h);
	}	
      }    
    }
  }else{
    if(k!=-1){
      VISIBILITY.removeKey(v);
      if(v<-1){
	int j,i,*v1;
	i=-(v+10);
	Tsnode* sn=(Tsnode*) AM.SuperNodes.get(i);
	sn->parts.reset();
	while(v1=(int*) sn->parts.getnext()){
	  j=getI(*v1);
	  setVisibility(j,h);
	}	
      }   
    }
  }
}


Graph*
Graph::getSubgraph(List& vertices){
 Graph *G;
 int i,x,y,v1,v2,e1,l,n,d;
 double *values;
 int* v;

  G=new Graph;

  // copy vertices;
    
  for(x=0;x<vertices.count();x++){
    v=(int*) vertices.get(x);
    v1=getI(*v,l,values,n);
    G->set(v1,l,values,n);
  }

  // copy edges

  G->done(1);
  
  int *ind_x,*ind_y;
  for(x=0;x<G->numberOfVertices();x++){
    v1=G->getI(x);
    ind_x=(int*) Vertices.getValue(v1);
    for(y=0;y<G->numberOfVertices();y++){
      v2=G->getI(y);
      d=getNumberOfEdges(v1,v2);
      ind_y=(int*) Vertices.getValue(v2);
      for(i=0;i<d;i++){
	e1=getEdgesBetween(*ind_x,*ind_y, i,l,values,n);
	G->setEdge(v1,v2,e1,l,values,n);
      }
    }
  }
  G->setName(name);

  // copy statstical information
  G->AM.edge_access=AM.edge_access;
  
  return G; 
}




void 
Graph::generateDistortions(SortedList& distList,double max_error){

  //consider only edge insertions and deletions in the beginning

  int x,y,d,e,edg,n,label;
  double *values;
  Edit_type* E;
  AttId at,i_at;
  
  // take the first attribute that appears in the graph as 
  // the template for future insertions ... this is clearly
  // a hack if you ever seen one

  e=0;
  for(x=0;x<numberOfVertices();x++){
    for(y=0;y<numberOfVertices();y++){
      e=getEdgeNR(x,y);
      if(e>0){
	AM.initNext(x,y);
	i_at=AM.isNextEdgeAttId(&edg);
	break;
      }
    }
    if(e>0) break;
  }

  for(x=0;x<numberOfVertices();x++){
    if(!AM.isDirected()) d=x+1;
    else d=0;

    for(y=d;y<numberOfVertices();y++){
      if(x==y) continue;

      E=new Edit_type;

      e=getEdgeNR(x,y);
      if(e>0){
	AM.initNext(x,y);
	at=AM.isNextEdgeAttId(&edg);
	E->type=1;
	E->x=x;
	E->y=y;
	E->cost=ATT_object.deletionCostOfEdge(at,x,y,this);

      }else{
	E->type=0;
	E->x=x;
	E->y=y;
	E->cost=ATT_object.insertionCostOfEdge(i_at,x,y,this);
	E->label=i_at.Label;
	E->values=i_at.values;
	E->n=i_at.n;
      }
      distList.insert(E,0,E->cost);
    }
  }
}
  


Graph* 
Graph::applyEdits(Edit_type* E){

Graph *G;
 int i,x,y,v1,v2,e1,l,n,d,edg;
 double *values,*e_values;;
 int* v;

  G=new Graph;

  // copy vertices;
    
  for(x=0;x<numberOfVertices();x++){
    v1=getI(x,l,values,n);
    e_values=values;
    G->set(v1,l,values,n);
  }

  // copy edges

  G->done(AM.isDirected());
  
  int dir;
  edg=0;
  int *ind_x,*ind_y;
  for(x=0;x<numberOfVertices();x++){
    v1=getI(x);
   
    if(!AM.isDirected()) dir=x+1;
    else dir=0;

    for(y=dir;y<numberOfVertices();y++){

      v2=getI(y);
      d=getNumberOfEdges(v1,v2);

      if(d>0){
	if((E->type==1)&&(E->x==x)&&(E->y==y)) continue;

	for(i=0;i<d;i++){
	  e1=getEdgesBetween(x,y, i,l,values,n);
	  G->setEdge(v1,v2,edg++,l,values,n);
	}
      }else{
	if((E->type==0)&&(E->x==x)&&(E->y==y)){
	  
	  G->setEdge(v1,v2,edg++,E->label,e_values,E->n);
	}
      }
    }
  }
  G->setName(name);

  // copy statstical information
  G->AM.edge_access=AM.edge_access;
  
  return G; 
  
}




int
Graph::visible(int v){
  int k;
  
  k=VISIBILITY.getValueKey(v);
  if(k!=-1)
    return 0;
  else{

    if(v<-1){
      int j,i,*v1;
      i=-(v+10);
      Tsnode* sn=(Tsnode*) AM.SuperNodes.get(i);
      sn->parts.reset();
      while(v1=(int*) sn->parts.getnext()){
	j=getI(*v1);
	if(!visible(j)) return 0;
      }	
    }   

    return 1;
  }
}


Graph*
Graph::getVisibleGraph(){
  Graph *G;
  int i,x,y,v1,v2,e1,l,n,d;
  double *values;

  G=new Graph;

  // copy vertices;
    
  // no SuperNodes included

  for(x=0;x<AM.Dimension;x++){
    v1=getI(x,l,values,n);
    if(visible(v1))
      G->set(v1,l,values,n);
  }

  // copy edges

  G->done(1);
  
  int *ind_x,*ind_y;
  for(x=0;x<G->numberOfVertices();x++){
    v1=G->getI(x);
    ind_x=(int*) Vertices.getValue(v1);
    for(y=0;y<G->numberOfVertices();y++){
      v2=G->getI(y);
      d=getNumberOfEdges(v1,v2);
      ind_y=(int*) Vertices.getValue(v2);
      for(i=0;i<d;i++){
	e1=getEdgesBetween(*ind_x,*ind_y, i,l,values,n);
	G->setEdge(v1,v2,e1,l,values,n);
      }
    }
  }
  G->setName(name);

  // copy statstical information
  G->AM.edge_access=AM.edge_access;
  
  return G;
}


List*
Graph::getMinimalCycles(int length){
  int x,y,level,n,n1,n2,edg;
  List *RET;
  SortedList *cl;
  Hash touch;
  SortedList OPEN,CLOSED[2];
  int *v;
  int circuit=0;
  AttId at;
  
  struct NT{
    int v;
    int spoint;
    NT* parent;
  };

  NT *node,*node2,*exit_node;

  RET=new List;

/*

  SuperNodes are not to be included in the cycles, therefore
  we let FOR run to Dimension not numberOfVertices

*/

  
//  for(x=0;x<AM.numberOfVertices();x++){

  for(x=0;x<AM.Dimension;x++){

    if(touch.in(x)) continue;

    // init OPEN
   
    level=0;
    node=new NT;
    node->v=x;
    node->spoint=0;
    node->parent=NULL;
    OPEN.insert(node,level,level);

    exit_node=NULL;
    while((OPEN.count()>0)&&(exit_node==NULL)){

      node=(NT*) OPEN.removeTop(&level);
      
      if(level>length){
	delete node;
	continue;
      }

      // was v already visited from this direction ?
      if(CLOSED[node->spoint].getValue(node->v)){
	delete node;
	continue;
      }
      
      CLOSED[node->spoint].insert(node,level,node->v);

      AM.initNext(node->v,x);
      if((at=AM.isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
	if(at.values[5]!=node->spoint)
	  if(at.values[6]==0){
	    exit_node=node;
	    break;
	  }
      }

      AM.initNext(node->v);
      
      while((edg=AM.getnext(&n1,&n2,&at))!=-1){
	if(at.values[5]!=(node->spoint)){
	  
	  if(n1==node->v){
	    
	    if(n2!=x){
	      node2=new NT;	
	      node2->v=n2;
	      node2->parent=node;
	      node2->spoint=at.values[6];
	      OPEN.insert(node2,level+1,level+1);
	    }
	  }
	}
      }
    }

    // follow back

    if(exit_node!=NULL){
      cl=new SortedList;
      node=exit_node;
      n=0;
      while(node!=NULL){
	
	touch.insert(NULL,node->v);
	
	n=getI(node->v);
	cl->insert(NULL,n,node->v);
	node=node->parent;
      }      
      RET->insert(cl);
    }

    
    exit_node=NULL;
    
    CLOSED[0].clearDeep();
    CLOSED[1].clearDeep();
    OPEN.clearDeep();
  }
  
  if(RET->count()>0)
    return RET;
  else{
    delete RET;
    return NULL;
  }
  
}


void
Graph::expandableEdges(int f){
  AM.expandable_edges_flag=f;
}


void
Graph::generateSuperNodes(double threshold,double weight){

// calling external function 

#ifdef SUPERNODES  
  generateSuperNodes_ex(this, threshold, weight);
#endif
}

#ifdef SUPERNODES  
int SUPERNODES_INDICATOR;
#endif

void
Graph::deleteSuperNodes(){

  AM.deleteSuperNodes();

}
