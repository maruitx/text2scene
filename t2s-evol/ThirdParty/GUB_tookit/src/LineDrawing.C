#include "LineDrawing.h"

static int ENV_VERTEX_EDGE_GRAPH=0;
static int ENV_UNDIRECTED_GRAPH=0;
static int ENV_VERTEX_EDGE_ANGLE_GRAPH=1;
static int ENV_PARALLEL_ATTRIBUTE=0;
static int ENV_EDGE_GRAPH=1;
static int ENV_NO_ART_LINES=1;
static double PARALLEL_THRESHOLD=0.1;
static int ENV_ALL_SPATIALS=0;

static FILE* GFile=NULL;

char GlobalName[256];

int extract_from_robmann_file(char* name,List& line_list,double line_threshold){
 char what;
 char out[256];
 int x,x1,x2,y1,y2,i1,i2;
 int v_dim,e_dim;
 FILE* file;
 List LINES;
 line *L;
 Graph *G;
 char* env;
 point *P,*P1,*P2;
 SortedList SL;
 
 file=fopen(name,"r");
 
 if(!file) return 0;
 

 v_dim=(int) get_zahl(file);
 e_dim=(int) get_zahl(file);
 
 for(x=0;x<v_dim;x++){
   P=new point;
//   P->x=(int) get_zahl(file);
//   P->y=(int) get_zahl(file);
   P->y=(int) get_zahl(file);
   P->x=(int) get_zahl(file);

   SL.insert(P,x+1,x+1);
 }

 int m=0;
 for(x=0;x<SL.count();x++){
   P=(point*) SL.get(x);
   if(m<P->y) m=(int) P->y;
 }

 for(x=0;x<e_dim;x++){
   i1=(int) get_zahl(file);
   i2=(int) get_zahl(file);
   P1=(point*) SL.getKeyData(i1);
   P2=(point*) SL.getKeyData(i2);
   L=new line;
   L->set(P1->x,m-P1->y,P2->x,m-P2->y);
   line_list.insert(L);
 } 

 fclose(file);
 return 1;
}



int extract_from_eepic_file(char* name,List& LINES,double line_threshold){
 char what;
  char out[256];
  int x,x1,x2,y1,y2;
  int lineDim;
  FILE* file;
 
  line *L;
  Graph *G;
  char* env;
  char* text,buf[256];
  double scale;

  file=fopen(name,"r");

  if(!file) return 0;

  lineDim=0;

  what=getc_c(file);

  while(!feof(file)){
   
    switch(what){
      
    case '\\': break;
    case 'p': 
      for(x=0;x<4;x++) what=getc_c(file);
      x1=(int) (get_zahl(file)/scale);
      y1=(int) (get_zahl(file)/scale);
      what=(char) (getc_c(file)/scale);
      x2=(int) (get_zahl(file)/scale);
      y2=(int) (get_zahl(file)/scale);
      
      L=new line;
      L->set(x1,y1,x2,y2);
      LINES.insert(L);
      lineDim++;
      break;
    case 'b':
 	
	fgets(buf,255,file);
	text=strtok(buf,"egin{picture}(");
	
	
	if(text){
	
	    if(text[0]=='o') break;
	    
	    int val1=atoi(text);
	    int val2=atoi(strtok('\0',",) \n"));
	    if(val2>val1) val1=val2;
	    if(val1>1000) scale=6;
	    else scale=1;
	}
	break;
    case '\n': break;
    default: fgets(out,255,file);
    }	    
    what=getc_c(file);
    
  
  }
 fclose(file);

 return 1;

}






int extract_from_niggeler_open(FILE* file,List& LINES,double line_threshold){
 char what;
  char out[256];
  int x,x1,x2,y1,y2;
  int lineDim;

 
  line *L;
  Graph *G;
  char* env;
 int flag=0;
 
  lineDim=0;

 if(feof(file)) return 0;
 
 what=getc(file);

 

  while(!feof(file)){
   
    switch(what){
      
    case '\\': break;
    case 'b':
      if(flag){
	ungetc(what,file);
	return 1;
      }else{
	flag=1;
	fgets(GlobalName,255,file);
      }
      break;
    case '\n': break;
    default: 
      ungetc(what,file);
      x1=(int) get_zahl(file);
      y1=(int) get_zahl(file);
      x2=(int) get_zahl(file);
      y2=(int) get_zahl(file);
      
      x1=2*x1;
      x2=2*x2;
      y1=(150-2*y1);
      y2=(150-2*y2);

      L=new line;
      L->set(x1,y1,x2,y2);
      LINES.insert(L);
      lineDim++;
      break;
    
    }	    
    what=getc_c(file);
  }


 return 1;

}

int extract_from_niggeler_file(char* name,List& LINES,double line_threshold){
  FILE* file;
  int ret;
  file=fopen(name,"r");

  if(!file) return 0;
  ret=extract_from_niggeler_open(file, LINES, line_threshold);
  fclose(file);
  return ret;
}

void clusterEndpoints(List* LINES,double line_threshold);
Graph* linesToGraph(List* LINES,double line_threshold);


Graph* extract_line_graph_from_eepic2(char *name,double line_threshold){

  return extract_line_graph_from_eepic(name,line_threshold,4,0);
}


Graph* extract_line_graph_from_eepic(char *name,double line_threshold, double dist_threshold, double connect_threshold){
 

  FILE* file;
  List LINES;
 
  Graph *G;
  char* env;

  env=getenv("ENV_NO_ART_LINES");
  if(env!=NULL) ENV_NO_ART_LINES=1;
  env=getenv("ENV_VERTEX_EDGE_GRAPH");
  if(env!=NULL) ENV_VERTEX_EDGE_GRAPH=1;  // transform vertices into graph vertices
  env=getenv("ENV_UNDIRECTED_GRAPH");
  if(env!=NULL) ENV_UNDIRECTED_GRAPH=1;   // transform into undirected graph
  env=getenv("ENV_VERTEX_EDGE_ANGLE_GRAPH");
  if(env!=NULL) ENV_VERTEX_EDGE_ANGLE_GRAPH=1;  
  env=getenv("ENV_PARALLEL_ATTRIBUTE");
  if(env!=NULL) {
    ENV_PARALLEL_ATTRIBUTE=1;
    PARALLEL_THRESHOLD=atof(env);
  }
  env=getenv("ENV_EDGE_GRAPH");
  if(env!=NULL) ENV_EDGE_GRAPH=1;
  
  env=getenv("ENV_ALL_SPATIALS");
  if(env!=NULL) ENV_ALL_SPATIALS=1;

  if(strstr(name,".rob")!=NULL){
    extract_from_robmann_file(name,LINES,line_threshold);
    ENV_PARALLEL_ATTRIBUTE=1;
  }else if(strstr(name,".nig")!=NULL){
    extract_from_niggeler_file(name,LINES,line_threshold);  
  }else if(strstr(name,".mul")!=NULL){
    if(GFile==NULL){
      GFile=fopen(name,"r");
    }

    if(feof(GFile)) return NULL;
    if(!extract_from_niggeler_open(GFile,LINES,line_threshold))
      return NULL;
    
  }else{
    extract_from_eepic_file(name,LINES,line_threshold);
  }


  if(LINES.count()==0) return NULL;

  split_lines_at_intersections(&LINES, line_threshold);
  
 


#ifdef OLD_ENCODING
  
  delete_tiny_lines(&LINES,line_threshold);

  G=make_line_graph(&LINES);

#else

  clusterEndpoints(&LINES, dist_threshold);
  
  delete_tiny_lines(&LINES,line_threshold);

  G=linesToGraph(&LINES, connect_threshold);

#endif

  return G;
  
}



void split_lines_at_intersections(List *LINES,double threshold){
  int x,y;
  List TEMP,TEMP2;
  List Lost;
  line *l1,*l2,*new_a,*new_b;
  int change;
  change=1;

  TEMP=*LINES;
 
 
  for(x=0;x<LINES->count();x++){
    l1=(line*) LINES->get(x);
    for(y=0;y<TEMP.count();y++){
      l2=(line*) TEMP.get(y);
      if(l1->intersect(*l2,threshold)){

	new_a=new line;
	new_a->set(l1->CP.x,l1->CP.y,l2->l.x,l2->l.y);
	new_b=new line;
	new_b->set(l1->CP.x,l1->CP.y,l2->r.x,l2->r.y);

	if(new_a->length()>0)
	  TEMP2.insert(new_a);	
	else
	    delete new_a;

	if(new_b->length()>0)
	  TEMP2.insert(new_b);
	else
	    delete new_b;
	
	Lost.insert(l2);
      }else{      
	TEMP2.insert(l2);
      }
    }
    
    TEMP=TEMP2;
    TEMP2.clear();
  }
  
  while(Lost.count()>0){
    l2=(line*) Lost.remove(0);
    delete l2;
  }
  (*LINES)=TEMP;
}



void delete_tiny_lines(List* LINES,double line_threshold){
  int x;
  List TEMP;
  line *l;

  for(x=0;x<LINES->count();x++){
    l=(line*) LINES->get(x);
    if(l->length()>line_threshold){
      TEMP.insert(l);
    }else{
      delete l;
    }
  }
  (*LINES)=TEMP;
}



Graph* make_line_graph(List *LINES){
  int x,i,e;
  line *l;
  double *values;
  int vn,en,vlabel,elabel;
  

  Graph *G;
  G=new Graph;
 
  vn=2;
  en=0;
  elabel=1;
 
  values=new double[vn];
  i=0;
  e=0;
  for(x=0;x<LINES->count();x++){
    l=(line*) LINES->get(x);
    values[0]=(double) l->r.x;
    values[1]=(double) l->r.y;
    vlabel=6;
    vn=2;
    G->set(i++,vlabel,values,vn);
    values[0]=(double) l->l.x;
    values[1]=(double) l->l.y;
    G->set(i++,vlabel,values,vn);
    G->setEdge(i-2,i-1,e++,elabel,NULL,0);
  }
  delete values;
  G->doOnlyIdenticalEdges();
  G->done(0);
  
  return G;

}


#define quad(x) ((x)*(x))

struct npoint{
  point p;
  npoint* next;
};

int mergePoints(npoint* &pl,npoint* &ql,double threshold){
  point m;
  double d;
  
  if(pl==ql) return 0;

  d=quad(pl->p.x-ql->p.x)+quad(pl->p.y-ql->p.y);
  if((d<threshold)){
    m.x=(pl->p.x+ql->p.x)/2;
    m.y=(pl->p.y+ql->p.y)/2;
    
    pl->next=new npoint;
    pl=pl->next;
    
    pl->p=m;
    ql->next=pl;
    ql=pl;
    
    pl->next=NULL;
    return 1;
  }else{
    return 0;
  }
}


void clusterEndpoints(List* LINES, double th){
  int x,y,nx,ny;
  int change,move;
  List tmp;
  line* l1,*l2;
  double d,threshold;
  int mx,my;
  line** Vec;
  
  npoint** VP;
  npoint* pl,*pr,*ql,*qr;

  if(LINES->count()<2) return;

  Vec=new line*[LINES->count()];
  VP=new npoint*[LINES->count()*2];

  for(x=0;x<LINES->count();x++){
    Vec[x]=(line*) LINES->get(x);
    VP[2*x]=new npoint;
    VP[2*x]->p=Vec[x]->l;
    VP[2*x]->next=NULL;
    VP[2*x+1]=new npoint;
    VP[2*x+1]->p=Vec[x]->r;
    VP[2*x+1]->next=NULL;
  }

  threshold=th*th;
  
  
  for(x=0;x<LINES->count();x++){
    
    pl=VP[2*x];
    while(pl->next!=NULL)
      pl=pl->next;
    
    pr=VP[2*x+1];
    while(pr->next!=NULL)
      pr=pr->next;
      
    
    for(y=0;y<LINES->count();y++){
      
      ql=VP[2*y];
      while(ql->next!=NULL)
	ql=ql->next;
      
      qr=VP[2*y+1];
      while(qr->next!=NULL)
	qr=qr->next;
	
      mergePoints(pl,ql,threshold);
      mergePoints(pl,qr,threshold);
      mergePoints(pr,ql,threshold);
      mergePoints(pr,qr,threshold);
      
    }
  }

// reconstruct lines with merged points
  
  for(x=0;x<LINES->count();x++){
    pl=VP[2*x];
    while(pl->next!=NULL)
      pl=pl->next;
    
    pr=VP[2*x+1];
    while(pr->next!=NULL)
      pr=pr->next;
    
    Vec[x]->set(pl->p.x,pl->p.y,pr->p.x,pr->p.y);
  
  }

  List Trash;
  for(x=0;x<(LINES->count()*2);x++){
    pl=VP[x];
    while(pl->next!=NULL){
      if(pl->p.x!=-30000){
	Trash.insert(pl);
	pl->p.x=-30000;
	pl=pl->next;
      }else{
	break;
      }
    }
  }

  while(Trash.count()>0){
    pl=(npoint*) Trash.remove(0);
    delete pl;
  }
  delete[] VP;
  delete[] Vec;

}



Graph* linesToGraph(List *LINES, double dist_threshold){
  int x,i,e;
  line *l,*l2;
  double values[8];
  int vn,en,vlabel,elabel;
  line** Vec;

  Graph *G;
  G=new Graph;
 
  vn=6;
  en=0;
  elabel=1;
 
  
  i=0;
  e=0;
  Vec=new line*[LINES->count()];

  for(x=0;x<LINES->count();x++){
    l=(line*) LINES->get(x);
    values[0]=(double) l->l.x;
    values[1]=(double) l->l.y;
    values[2]=(double) l->r.x;
    values[3]=(double) l->r.y;
    values[4]=l->length();
    values[5]=l->degree();
    vlabel=0;
    vn=6;
    G->set(i++,vlabel,values,vn);
  
    Vec[x]=l;

  }

  G->doOnlyIdenticalEdges();
  G->done(1);

  // construct edges (connections between linesegments)


  int max_edges=0;
  int y,touch_1,touch_2;
  double sig,alpha,alpha2,ratio;

  for(x=0;x<LINES->count();x++){
    l=Vec[x];
    
    for(y=x+1;y<LINES->count();y++){
      l2=Vec[y];
      
      touch_1=5;
      touch_2=5;

      if(((l->l.x==l2->l.x)&&(l->l.y==l2->l.y))){
	touch_1=0;touch_2=0;
      }

      if((l->l.x==l2->r.x)&&(l->l.y==l2->r.y)){
	touch_1=0;touch_2=1;
      }

      if((l->r.x==l2->r.x)&&(l->r.y==l2->r.y)){
	touch_1=1;touch_2=1;
      }

      if((l->r.x==l2->l.x)&&(l->r.y==l2->l.y)){
	touch_1=1;touch_2=0;
      }

      if(touch_1+touch_2<5){
	
	alpha=l->angle(l2,sig);
	if(alpha>PI/2) alpha2=sig*(PI-alpha);
	else alpha2=sig*alpha;
	ratio=(l->length()-l2->length())/l2->length();
	
	values[0]=floor(100*alpha)/100;
	values[1]=floor(100*ratio)/100;
	values[2]=alpha2;
	values[3]=l->length();
	values[5]=touch_1;
	values[6]=touch_2;

	G->setEdge(x,y,max_edges++,EDGE_ANGLE_LABEL,values,7);
	
	ratio=(l2->length()-l->length())/l->length();
	values[1]=floor(100*ratio)/100;
	values[2]=-alpha2;
	values[3]=l2->length();
  
	values[5]=touch_2;
	values[6]=touch_1;

	G->setEdge(y,x,max_edges++,EDGE_ANGLE_LABEL,values,7);
      }else{
	
	// not touching

	if(ENV_ALL_SPATIALS){
	  
	  line hl,hl2,hl3;
	  
	  l->intersect(*l2,dist_threshold);
	  
	  hl.set(l->l.x,l->l.y,l->CP.x,l->CP.y);
	  hl3.set(l->r.x,l->r.y,l->CP.x,l->CP.y);
	  if(hl.length()<hl3.length())
	    hl=hl3;
	  
	  hl2.set(l2->l.x,l2->l.y,l->CP.x,l->CP.y);
	  hl3.set(l2->r.x,l2->r.y,l->CP.x,l->CP.y);
	  if(hl2.length()<hl3.length())
	    hl2=hl3;

	  
	  alpha=hl.angle(&hl2,sig);
	  if(alpha>PI/2) alpha2=(PI-alpha);
	  else alpha2=alpha;
	  
	  values[0]=alpha2;
	  
	  G->setEdge(x,y,max_edges++,SPATIAL_LABEL,values,1);
	  
	  G->setEdge(y,x,max_edges++,SPATIAL_LABEL,values,1);
	  
	}	
      }
    }
  } 

  
  return G;

}

#ifdef OLD_ENCODING
Graph* build_complex_vertices(Graph* G, double radius_threshold,double angle_threshold){
  int x,y,i,d,n;
  int* v;
  List LL;
  List* l;
  int* bit;
  double *ivalues,*values;
  int label,ilabel;

  Graph *new_G;
  
  d=G->numberOfVertices();
  bit=new int[d];
  for(x=0;x<d;x++) bit[x]=0;
  
  for(x=0;x<d;x++){
    if(bit[x]) continue;
    
    bit[x]=1;
    
    l=new List;
    v=new int;
    *v=x;
    l->insert(v);
    i=0;
    while(i<l->count()){
    
      v=(int*) l->get(i);
      G->getI(*v,ilabel,ivalues,n);

      for(y=x+1;y<d;y++){
	int* w;
	double dist;

	if(bit[y]) continue;
	G->getI(y,label,values,n);
	dist=(ivalues[0]-values[0])*(ivalues[0]-values[0])+(ivalues[1]-values[1])*(ivalues[1]-values[1]);
	if(dist<radius_threshold*radius_threshold+3){
	  w=new int;
	  *w=y;
	  l->insert(w);
	  bit[y]=1;
	}
      }
      i++;
    }
    LL.insert(l);
  }

  delete bit;

  /* collect new graph vertices  */
  /* and insert all edges        */

  new_G=new Graph;

  LL.reset();
  i=0;
  label=6;
  values=new double[2];

  while(l=(List*) LL.getnext()){
    
    v=(int*) l->get(0);
    
    G->getI(*v,label,ivalues,n);
   
    if(n>1){
      values[0]=ivalues[0];
      values[1]=ivalues[1];
    }
  
    
    new_G->set(i++,label,values,2);
    //new_G->set(i++,label,values,label);
    

  }
  delete values;

  new_G->done(0);

  // collect edges: this is still a intersection-edge-intersection graph!

  LL.reset();
  i=0;
  int j,jj,ii,*w,e;
  List* l2;

  while(l=(List*) LL.getnext()){
     
    for(j=i+1;j<LL.count();j++){
      if(i==j) continue;
      l2=(List*) LL.get(j);
      
      for(ii=0;ii<l->count();ii++){
	v=(int*) l->get(ii);

	for(jj=0;jj<l2->count();jj++){
	  w=(int*) l2->get(jj);

	  if(G->getEdgeNR(*v,*w)>0){
	    if(new_G->getEdgeNR(i,j)==0){
	      e=G->getEdgesBetween(*v,*w, 0, label, values, n);
	      new_G->setEdge(i,j,e,label,values,n);
	    }
	  }
	}
      }
    }
    i++;
  }

/* clear LL and l  */

  while(LL.count()>0){
    l2=(List*) LL.remove(0);
    while(l2->count()>0){
      w=(int*) l2->remove(0);
      delete w;
    }
    delete l2;
  }
  
// stop here if all you want is a intersection-edge-intersecton graph!  

/* create artificial edges, if two vertices are connected by kollinear edges */

  if(!ENV_NO_ART_LINES)
    create_additional_edges(new_G,angle_threshold);

 
/**************************************/
/* Create vertex-edge-vertex Graph    */
/* each edge in new_G becomes a vertex*/
/* which will have edges to the old   */
/* vertices                           */
/* and edges to the neighbouring edges*/
/**************************************/


  line l_a,*l_b;
  int e_nr,xt,yt,in,p1,p2,offset;
  double *new_values,edge_values[2];
  double alpha,ratio;
  SortedList order,Realized;
  int edge,edge1;
  Graph* new2_G;

  

  if(ENV_VERTEX_EDGE_ANGLE_GRAPH){
    new2_G=extract_vertex_edge_angle_graph(new_G);

    if(ENV_PARALLEL_ATTRIBUTE){
      Graph* new3_G;

      new3_G=extract_parallel_line_graph(new2_G);
      delete new2_G;
      new2_G=new3_G;
    }


    // delete new_G;

  }else{
    
    new2_G=new Graph;
    for(x=0;x<new_G->numberOfVertices();x++){
      
      i=0;
      xt=new_G->getI(x,label,values,n);
      new_values=new double[n];
      for(y=0;y<n;y++)
	new_values[y]=values[y];
      
      e_nr=new_G->getEdgeNR(xt);
      
      edge1=new_G->getEdgesAdjacentTo(xt,i++,p1,ilabel,ivalues,in);
      new_G->get(p1,ilabel,ivalues,in);
      l_a.set(values[0],values[1],ivalues[0],ivalues[1]);
      for(y=1;y<e_nr;y++){
	l_b=new line;
	edge=new_G->getEdgesAdjacentTo(xt,i++,p2,ilabel,ivalues,in);
	new_G->get(p2,ilabel,ivalues,in);
	l_b->set(values[0],values[1],ivalues[0],ivalues[1]);
	alpha=l_a.angle(l_b);
	if(alpha<0) alpha=-alpha;
	order.insert(l_b,edge,alpha);
	
      }
      
      // Last angle must also be represented
      l_b=new line;
      *l_b=l_a;
      
      order.insert(l_b,edge1,0);
      
      
      y=2;
      
      if(ENV_VERTEX_EDGE_GRAPH)
	new2_G->set(xt,label,new_values,n);
      offset=new_G->maximumVertex()+1;
      
      double e_values[6];
      if(!Realized.isKey(edge1)){
	e_values[0]=l_b->length();
	e_values[1]=l_b->degree();
	e_values[2]=l_b->l.x;
	e_values[3]=l_b->l.y;
	e_values[4]=l_b->r.x;
	e_values[5]=l_b->r.y;
	new2_G->set(edge1+offset,EDGE_AS_VERTEX_LABEL,e_values,6);
	Realized.insert(NULL,edge1,edge1);
      }
      
      if(ENV_VERTEX_EDGE_GRAPH)
	new2_G->setEdge(xt,offset+edge1,-1,VERTEX_TO_EDGE_LABEL,NULL,0);
      
      
      while(order.count()>0){
	l_b=(line*) order.removeTop(&edge);
	if(!Realized.isKey(edge)){
	  e_values[0]=l_b->length();
	  e_values[1]=l_b->degree();
	  e_values[2]=l_b->l.x;
	  e_values[3]=l_b->l.y;
	  e_values[4]=l_b->r.x;
	  e_values[5]=l_b->r.y;
	  new2_G->set(edge+offset,EDGE_AS_VERTEX_LABEL,e_values,6);
	  Realized.insert(NULL,edge,edge);
	}
	
#if 1
	if(order.count()==0) break;
	line* l_c;
	order.initNext();
	while(l_c=(line*) order.getnext(&edge1)){
	  if(!Realized.isKey(edge1)){
	    e_values[0]=l_c->length();
	    e_values[1]=l_c->degree();
	    e_values[2]=l_c->l.x;
	    e_values[3]=l_c->l.y;
	    e_values[4]=l_c->r.x;
	    e_values[5]=l_c->r.y;
	    new2_G->set(edge1+offset,EDGE_AS_VERTEX_LABEL,e_values,6);
	Realized.insert(NULL,edge1,edge1);
	  }
	  
	  
	  
	  alpha=l_c->angle(l_b);
      
	  if(alpha<0) alpha=-alpha;
	  if(alpha>PI) alpha=2*PI-alpha;
	  
	ratio=l_c->length()/l_b->length();
	  edge_values[0]=alpha;
	  edge_values[1]=ratio;
	  
	  
	new2_G->setEdge(offset+edge,offset+edge1,-1,EDGE_TO_EDGE_LABEL,edge_values,2);
	  if(!ENV_UNDIRECTED_GRAPH){
	    edge_values[1]=1/ratio;
	    new2_G->setEdge(offset+edge1,offset+edge,-1,EDGE_TO_EDGE_LABEL,edge_values,2);
	  }
	}
	
	if(ENV_VERTEX_EDGE_GRAPH)
	  new2_G->setEdge(xt,offset+edge,-1,VERTEX_TO_EDGE_LABEL,NULL,0);
	edge1=edge;
      delete l_c;
	l_c=l_b;
	
	
#else
	
	alpha=l_a.angle(l_b);
	
	if(alpha<0) alpha=-alpha;
	if(alpha>PI) alpha=2*PI-alpha;
	
	
      ratio=l_a.length()/l_b->length();
	if(ratio>1) ratio=1/ratio;
	edge_values[0]=alpha;
	edge_values[1]=ratio;
	
	if(edge1!=edge){
	  new2_G->setEdge(offset+edge1,offset+edge,-1,EDGE_TO_EDGE_LABEL,edge_values,2);
	  if(!ENV_UNDIRECTED_GRAPH){
	    edge_values[1]=1/ratio;
	    new2_G->setEdge(offset+edge,offset+edge1,-1,EDGE_TO_EDGE_LABEL,edge_values,2);
	  }
	}
	
      if(ENV_VERTEX_EDGE_GRAPH)
	new2_G->setEdge(xt,offset+edge,-1,VERTEX_TO_EDGE_LABEL,NULL,0);
	edge1=edge;
	l_a=*l_b;
	delete l_b;
	
#endif      
      }
      
      
    }
    
    
    
    delete new_G;
    if(ENV_UNDIRECTED_GRAPH)
      new2_G->done(0);
    else
      new2_G->done(1);
  }
  
  return new2_G;
  
}

#else

Graph* build_complex_vertices(Graph* G, double radius_threshold,double angle_threshold){

  return G;
}

#endif


Graph* extract_vertex_edge_angle_graph(Graph* new_G){
  int max_edges;
  int x,y,i,ilabel,in,n,label,edge1;
  int l_label,l_n;
  double *l_values;
  line* l_b,*l_c;
  line l_a;
  int e_nr,edge,p2,e_nr2;
  int p1;
  SortedList order;
  int xt,dir1,dir2;
  double * ivalues,*values;
  double new_values[10];
  double alpha,ratio,sig,alpha2;
  double e_values[8];
  Graph* new2_G;
  SortedList Done_list,In_this_loop;

  max_edges=0;
  new2_G=new Graph;
 
  if(!ENV_EDGE_GRAPH){
    gmt_assert(0);
    for(x=0;x<new_G->numberOfVertices();x++){ 
      i=0;
      xt=new_G->getI(x,label,values,n);
      //new_values=new double[n];
      for(y=0;y<n;y++)
	new_values[y]=values[y];
      new2_G->set(xt,label,new_values,n);
    }
  }

  
  for(x=0;x<new_G->numberOfVertices();x++){
    
    i=0;
    xt=new_G->getI(x,label,values,n);
    gmt_assert(n);
    //new_values=new double[n];
    for(y=0;y<n;y++)
      new_values[y]=values[y];
    
    e_nr=new_G->getEdgeNR(xt);
    
    edge1=new_G->getEdgesAdjacentTo(xt,i++,p1,ilabel,ivalues,in);
    new_G->get(p1,ilabel,ivalues,in);
    e_nr2=new_G->getEdgeNR(p1);
    l_b=new line;
    l_b->setKey(edge1);
    l_b->set(values[0],values[1],ivalues[0],ivalues[1]);
    if(ENV_EDGE_GRAPH)
      order.insert(l_b,p1,e_nr+e_nr2);
    else
      order.insert(l_b,p1,0);
    
      
    l_a=*l_b;
    for(y=1;y<e_nr;y++){
      l_b=new line;
      edge=new_G->getEdgesAdjacentTo(xt,i++,p2,ilabel,ivalues,in);
      new_G->get(p2,ilabel,ivalues,in);
      e_nr2=new_G->getEdgeNR(p2);
      l_b->set(values[0],values[1],ivalues[0],ivalues[1]);
      l_b->setKey(edge);
      alpha=l_a.angle(l_b);
      if(alpha<0) alpha=-alpha;

      if(ENV_EDGE_GRAPH)
	order.insert(l_b,p2,e_nr+e_nr2);
      else
	order.insert(l_b,p2,alpha);
    }
    
    
    double evals[6];
    line horz;

    if(ENV_EDGE_GRAPH) n=0;

    if((order.count()==1)||(n==2)){
      
      while(order.count()>0){
	e_nr2=(int) order.TopValue();
	 l_b=(line*) order.removeTop(&p2);
	 if(ENV_EDGE_GRAPH){
	   if(!Done_list.isKey(l_b->key())){
	     Done_list.insert(NULL,l_b->key(),0);
	     evals[0]=l_b->l.x;
	     evals[1]=l_b->l.y;
	     evals[2]=l_b->r.x;
	     evals[3]=l_b->r.y;
	     evals[4]=e_nr2;

	     if(l_b->l.y<l_b->r.y)
		horz.set(l_b->l.x,l_b->l.y,l_b->r.x,l_b->l.y);
	      else
		horz.set(l_b->r.x,l_b->r.y,l_b->l.x,l_b->r.y);
	      evals[5]=l_b->angle(&horz);
	      if(evals[5]>(PI/2)) evals[5]=PI-evals[5];

	     new2_G->set(l_b->key(),LABEL0,evals,6);
	   }else{
  /**  if l_b is already in Done list, then the direction of the edges is 1 **/
	   
	   }
	 }else
	   new2_G->setEdge(xt,p2,max_edges++,END_POINT_LABEL,NULL,0);
       }
    }else{
      
      while(order.count()>0){
	e_nr2=(int) order.TopValue();
	l_b=(line*) order.removeTop(&p2);
	
	if(ENV_EDGE_GRAPH){
	  if(!Done_list.isKey(l_b->key())){
	    Done_list.insert(NULL,l_b->key(),0);
	    
	    evals[0]=l_b->l.x;
	    evals[1]=l_b->l.y;
	    evals[2]=l_b->r.x;
	    evals[3]=l_b->r.y;
	    evals[4]=e_nr2;

	    if(l_b->l.y<l_b->r.y)
	      horz.set(l_b->l.x,l_b->l.y,l_b->r.x,l_b->l.y);
	    else
	      horz.set(l_b->r.x,l_b->r.y,l_b->l.x,l_b->r.y);
	    evals[5]=l_b->angle(&horz);
	    if(evals[5]>(PI/2)) evals[5]=PI-evals[5];
	    
	    new2_G->set(l_b->key(),LABEL0,evals,6);
	    dir1=0;
	  }else{
	    if(In_this_loop.isKey(l_b->key()))
	      dir1=0;
	    else
	      dir1=1;
	  }
	}

	for(i=0;i<order.count();i++){
	  l_c=(line*) order.get(i,p1,evals[4]);
	  alpha=l_c->angle(l_b,sig);
	  if(alpha>PI/2) alpha2=sig*(PI-alpha);
	  else alpha2=sig*alpha;

	  ratio=l_c->length()/l_b->length();
	  e_values[0]=alpha;
	  e_values[1]=ratio;
	  e_values[2]=alpha2;
	  e_values[3]=l_c->length();


	  new_G->get(p2,l_label,l_values,l_n);
	  e_values[4]=l_label;
//	  e_values[5]=l_c->incidencePoint(l_b);
//	  e_values[6]=l_b->incidencePoint(l_c);
	  e_values[5]=dir1;
	  
  
	  if(ENV_EDGE_GRAPH){
	    if(!Done_list.isKey(l_c->key())){
	      In_this_loop.insert(l_c,l_c->key(),l_c->key());
	      Done_list.insert(NULL,l_c->key(),0);
	      evals[0]=l_c->l.x;
	      evals[1]=l_c->l.y;
	      evals[2]=l_c->r.x; 
	      evals[3]=l_c->r.y;
	      
	      if(l_c->l.y<l_c->r.y)
		horz.set(l_c->l.x,l_c->l.y,l_c->r.x,l_c->l.y);
	      else
		horz.set(l_c->r.x,l_c->r.y,l_c->l.x,l_c->r.y);
	      evals[5]=l_c->angle(&horz);
	      if(evals[5]>(PI/2)) evals[5]=PI-evals[5];

	      new2_G->set(l_c->key(),LABEL0,evals,6);
	      dir2=0;
	    }else{
	      if(In_this_loop.isKey(l_c->key()))
		dir2=0;
	      else
		dir2=1;
	    }
	    e_values[6]=dir2;
	    new2_G->setEdge(l_b->key(),l_c->key(),max_edges++,EDGE_ANGLE_LABEL,e_values,7);
	  }else{
	    if(alpha!=0)
	      new2_G->setEdge(xt,p2,max_edges++,EDGE_ANGLE_LABEL,e_values,7);
	    
	  }
	  alpha=l_b->angle(l_c,sig);
	  if(alpha>PI/2) alpha2=sig*(PI-alpha);
	  else alpha2=sig*alpha;
	  
	  ratio=l_b->length()/l_c->length();
	  e_values[0]=alpha;
	  e_values[1]=ratio;
	  e_values[2]=alpha2;
	  e_values[3]=l_b->length();


	  new_G->get(p2,l_label,l_values,l_n);
	  e_values[4]=l_label;
//	  e_values[5]=l_b->incidencePoint(l_c);
//	  e_values[6]=l_c->incidencePoint(l_b);
	  e_values[5]=dir2;
	  e_values[6]=dir1;
	  
	  if(ENV_EDGE_GRAPH){
	    if(!Done_list.isKey(l_c->key())){
	      In_this_loop.insert(l_c,l_c->key(),l_c->key());
	      Done_list.insert(NULL,l_c->key(),0);
	      evals[0]=l_c->l.x;
	      evals[1]=l_c->l.y;
	      evals[2]=l_c->r.x;
	      evals[3]=l_c->r.y;
	      new2_G->set(l_c->key(),LABEL0,evals,5);
	    }
	    new2_G->setEdge(l_c->key(),l_b->key(),max_edges++,EDGE_ANGLE_LABEL,e_values,7);
	  }else{
	    
	    if(alpha!=0)
	      new2_G->setEdge(xt,p1,max_edges++,EDGE_ANGLE_LABEL,e_values,7);
	  }
	}
	
	delete l_b;
      }
      In_this_loop.clear();
    }
  }
  
  Done_list.clear();
  new2_G->done(1);
  return new2_G;
    
}


Graph* extract_parallel_line_graph(Graph *new_G){
 int max_edges;
  int x,y,i,ilabel,in,n,label,edge1,x1,e_nr2;
  line* l_b,*l_c;
  line l_a;
  int e_nr,edge,p2;
  int p1;
  SortedList order;
  int xt,dir;
  double * ivalues,*values,*new_values;;
  double alpha,ratio,sig,alpha2;
  double e_values[5];
  Graph* new2_G;
 

  max_edges=0;
  new2_G=new Graph;
 
  for(x=0;x<new_G->numberOfVertices();x++){ 
    i=0;
    xt=new_G->getI(x,label,values,n);
    new_values=new double[n];
    for(y=0;y<n;y++)
      new_values[y]=values[y];
    new2_G->set(xt,label,new_values,n);
  }

 new2_G->done(1);

  for(x=0;x<new_G->numberOfVertices();x++){
    
    i=0;
    xt=new_G->getI(x,label,values,n);
    new_values=new double[n];
    for(y=0;y<n;y++)
      new_values[y]=values[y];
    
    e_nr=new_G->getEdgeNR(xt);
    
    for(x1=0;x1<e_nr;x1++){
      edge1=new_G->getEdgesAdjacentTo(xt,x1,p1,label,values,n,dir);

      if(dir<0) continue;

      if(n==0) continue;

      e_nr2=new_G->getNumberOfEdges(p1,xt);
    
      
      if(new2_G->getNumberOfEdges(xt,p1)==0)
	new2_G->setEdge(xt,p1,max_edges++,REAL_EDGE,NULL,0);

      

      for(y=0;y<e_nr2;y++){

	new_G->getEdgesBetween(p1,xt,y,ilabel,ivalues,in);

	if(in>0){
      
	  if(values[2]*ivalues[2]>0){
	    if(fabs(fabs(values[2])-fabs(ivalues[2]))<PARALLEL_THRESHOLD){
	      double val[2];
	      val[0]=1;
	      val[1]=values[3]/ivalues[3];
	      new2_G->setEdge(xt,p1,max_edges++,PARALLEL_EDGE,val,2);
	    }
	  }else{
	    if(fabs(fabs(values[2])-fabs(ivalues[2]))<PARALLEL_THRESHOLD){
	      double val[2];
	      val[0]=0;
	      val[1]=values[3]/ivalues[3];
	      new2_G->setEdge(xt,p1,max_edges++,PARALLEL_EDGE,val,2);
	    }
	  }
	}
      }
    }
  }

 return new2_G;

}


void create_additional_edges(Graph* G,double angle_threshold){
  int x,dim,j,y,i;
  int v1,v2,v3;
  double *values,*values2,*values3;
  int label;
  line l1,l2;
  int n1,edim,e1,label2,n,edim2,label3;
  int edges;
  int *VX;

  
  dim=G->numberOfVertices();
  edges=G->numberOfEdges();
  VX=new int[dim+1];
  for(x=0;x<dim;x++) VX[x]=0;

  for(x=0;x<dim;x++){
    v1=G->getI(x, label, values, n1);
    edim=G->getEdgeNR(v1);
    for(y=0;y<edim;y++){
      e1=G->getEdgesAdjacentTo(v1,y,v2,label2, values2, n);
      G->get(v2,label,values2,n);
      
      l1.set(values[0],values[1],values2[0],values2[1]);
      VX[v1]=1;

      int path;
      while((edim2=G->getEdgeNR(v2))>1){
	path=0;
	for(i=0;i<edim2;i++){
	  e1=G->getEdgesAdjacentTo(v2,i,v3,label3, values3, n);
	  if((VX[v3]==0)&&(label3!=ARTIFICIAL_EDGE)){
	    G->get(v3,label,values3,n);
	    l1.set(values[0],values[1],values2[0],values2[1]);
	    l2.set(values2[0],values2[1],values3[0],values3[1]);
	    if(fabs(PI-l1.angle(&l2))<angle_threshold){
	      if(!G->getNumberOfEdges(v1,v3)){
		G->setEdge(v1,v3,edges++,ARTIFICIAL_EDGE,NULL,0);
		path=1;
		break;
	      }
	    }
	    
	  }
	} 
	if(path==0) break;
	v2=v3;
	values2=values3;
      }
    }
  }
  delete VX;
}















