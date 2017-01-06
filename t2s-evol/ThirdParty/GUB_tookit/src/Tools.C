#include "Tools.h"
//#include "AdjazenzMatrix.h"
#include "MappingData.h"



const double FPepsilon=0.0000001;
extern AttributeClass ATT_object;

extern int ENV_EDGE_FORWARD_CHECKING_DISABLED;

char getc_c(FILE* file){
  char what;
  do{
    what=getc(file);
  }while(what==' ');
  return what;
};


double get_zahl(FILE* file){
  int e,decimal;
  double p,r;
  char what;
  int NEG;

  r=0;
  e=1;
  p=1;
  decimal=0;

  what=getc_c(file);

  NEG=1;
  if(what=='-') {
    NEG=-1;
    what=getc_c(file);
  }

  while((((int(what)-'0')<10)&&((int(what)-'0')>=0))||(what=='.')){
    if(what=='.'){
      decimal=1;
    }else{
   
      if(decimal){
	p=p*0.1;
	r=r+(int(what)-'0')*p;
      }else{
	r=r*10+int(what)-'0';
      } 
    }
    what=getc(file);
    if(feof(file)) break;
  };

  return NEG*r;
} 



int checkMatch(AttributeClass *ATT,
	       AdjazenzMatrix* A,
	       AdjazenzMatrix *I, 
	       MappingData *M,
	       double err){
  double se,ee,e;
  int edge;
  int x,y,out,in,mout,min;
  Token *m,*o;
  AttId at,at_m;

  o=M->Original();
  m=M->Image();

  gmt_assert(o->length()==A->numberOfVertices());
  se=0;
  for(x=0;x<o->length();x++){
    at=A->getNodeAttributeId((*o)[x]);
    
    if((*m)[x]<0){
      e=ATT->insertionCost(at);
    }else{
      at_m=I->getNodeAttributeId((*m)[x]);
      e=ATT->error(at,at_m);
    }
    se=se+e;
  }  
  cout << "\nNode Errors: " << se;
  ee=0;
  for(x=0;x<A->numberOfEdges();x++){
    at=A->getEdgeAttributeId(x,&out,&in);
    
    mout=M->originalNodeTo(out);
    min=M->originalNodeTo(in);

    I->initNext(mout,min);

    while((at_m=A->isNextEdgeAttId(&edge))!=NO_ATTRIBUTE){
      e=ATT->error(at,at_m);
      if(e>=0) break;
    }

    if(at_m==NO_ATTRIBUTE)
      e=ATT->insertionCost(at);
   
    ee=ee+e;
      
  }

  cout << "\nEdge Errors: " << ee;
  se=se+ee;
  cout << "\nTotal      : " << se;
  gmt_assert(se==err);
  return 1;
}

static int TIMESEC=0;

void initRand(){
  int stime;
  long ltime;

  if(getenv("TIMECLOCK")) TIMESEC=1;
  else TIMESEC=0;

  ltime=time(NULL);
  stime=(unsigned int) ltime/2;
  srand(stime);
}


const long CLK_MINE=10000;

//long elapsed_time(){
//   long t;
//   long p;
//   if(!TIMESEC)
//     t=((long) clock())/CLK_MINE;
//   else
//     t=time(&p);
//
//  return t;
//}

time_t elapsed_time(){
	time_t t;
	time_t p;
	if (!TIMESEC)
		t = ((time_t)clock()) / CLK_MINE;
	else
		t = time(&p);

	return t;
}



/* Determine the largest possible Clique of disjoint token        */
/* call for each row and subsequently for each column recursively */ 

void Clique(int i,
	    int* Best,
	    int* Current,
	    int& Besti,
	    int& Currenti,
	    int Lim,
	    int** mx){
  int x,y;
  int good;

  if(i==Lim) return;
  if((Lim-i+Currenti)<Besti) return;  // if current plus rest is smaller break here

  good=1;

  for(x=0;x<i;x++){
    if(Current[x]){
      if(mx[x][i]==0) {
	good=0;
	break;
      }
    }
  }
  if(good){
    Current[i]=1;
    Currenti++;
  }else{
    return;
  }


  if(Currenti>Besti){
    for(x=0;x<Lim;x++) Best[x]=Current[x];
    Besti=Currenti;
  }

  for(x=i+1;x<Lim;x++)
    Clique(x,Best,Current,Besti,Currenti,Lim,mx);

  Current[i]=0;
  Currenti--;
}
    
/* best first tree search for the determination   */
/* of the best edge-to-edge match                 */
/* The search depth is restricted to 32           */
/* Therefore the number of multiple edges between */
/* two vertices should not exceed 32              */


double HS(double** m, 
	  int dim_l,
	  int dim_r){

  return HS_smart(m,dim_l,dim_r,NULL);
  
}


double HS_smart(double** m, 
	  int dim_l,
	  int dim_r,int* path){

  long int u;
  int x,i;
  double ret_err;
  SortedList Open;
  List L;
  struct state{
    long int used;
    int position;
    double err;
    state* n;
  };

  ret_err=0;
  state *s,*next_s;

  if(dim_l==1){
    double err=0;

    for(x=0;x<dim_r-1;x++)
	err+=m[0][x];
    ret_err=err;
  }else{

    for(x=0;x<dim_r;x++){
      s=new state;
      s->used=1;
      s->position=x;
      s->n=NULL;
      if(x==(dim_r-1)) s->used=0;
      else s->used=s->used << x;
      s->err=m[0][x];
      Open.insert(s,1,s->err);      
    }

    while(Open.count()>0){ 
      s=(state*) Open.removeTop(&i);
      
      if(s->err>=INFTY_COST){
	ret_err=s->err;
	delete s;	
	break;
      }

      if(i==dim_l){
	ret_err=s->err;
	if(path){
	  state *h;
	  int j;
	  j=dim_l-1;
	  h=s;
	  while(h!=NULL){
	    path[j--]=h->position;
	    h=h->n;
	  }
	  while(L.count()>0){
	    h=(state*) L.remove(0);
	    delete h;
	  }
	}

	delete s;
	break;
      }
     
      if(i==dim_l-1){
	double err=0;
	next_s=new state;
	
	for(x=0;x<dim_r;x++){
	  u=1;
	  u=u<<x;
	  if((s->used & u)==0){
	    if(x!=(dim_r-1)){
	      err+=m[i][x];
	    }
	  }
	}
	next_s->used=s->used;
	next_s->err=err+s->err;
	next_s->position=0;
	if(path) {
	  next_s->n=s;
	  L.insert(s);
	}else{
	  delete s;
	}
	Open.insert(next_s,i+1,next_s->err);

      }else{
	for(x=0;x<dim_r;x++){
	  u=1;
	  u=u<<x;
	  
	  if((s->used & u)==0){
	    next_s=new state;
	    if(x==dim_r-1) next_s->used=s->used;
	    else next_s->used=u | s->used;
	    
	    next_s->err=s->err+m[i][x];
	    next_s->position=x;
	    if(path) next_s->n=s;
	    Open.insert(next_s,i+1,next_s->err);
	  }
	}
	if(path) L.insert(s);
	else delete s;
      }
    }
    
    while(Open.count()>0){
      s=(state*) Open.removeTop(&i);
      delete s;
    }
  }

  return ret_err;

}



void collect_edge_edits(int dim_l,int dim_r,int* path,int* edge_l,int* edge_r,double **m,List& SUB_list,List& DEL_list,List& INS_list){
  int x,y;
  double err;
  Edit_Desc* eds;

  for(x=0;x<dim_l-1;x++){
    y=path[x];
    m[dim_l-1][y]=0;
    if(m[x][y]>0){
      eds=new Edit_Desc;
      if(y==dim_r-1){
	eds->edge_o=edge_l[x];
	eds->err=m[x][y];
	DEL_list.insert(eds);
      }else{
	eds->edge_o=edge_l[x];
	eds->edge_i=edge_r[y];
	eds->err=m[x][y];
	SUB_list.insert(eds);
      }
    }
  }

  for(y=0;y<dim_r-1;y++){
    if(m[dim_l-1][y]>0){
      eds=new Edit_Desc;
      eds->edge_i=edge_r[y];
      eds->err=m[dim_l-1][y];
      INS_list.insert(eds);
    }
  }
}


#ifdef MULTIPLE_EDGES
double edgeErrorBetween2Vertices(AdjazenzMatrix* WModel, AttributeClass* ATTC, AttId* e_l,int dim_l,int l,int r,int dir){
  int x,y;
  AttId e_r[32];
  int dim_r;
  double **m;
  int edg;
  double ret_val;
  AttId at;
  
  dim_r=0;
  if((l>-1)&&(r>-1)){
    if(dir==1)
      WModel->initNext(l,r);
    else
      WModel->initNext(r,l);
    while((at<WModel->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
      e_r[dim_r++]<at;
    }
  }
#ifdef NEW_ATTOBJECT
  e_r[dim_r++].set(-1,NULL,0);
#else
  e_r[dim_r++]=-1;
#endif
	
  if(dim_l*dim_r==1) return 0;

  m=new double*[dim_l];
  for(x=0;x<dim_l;x++) m[x]=new double[dim_r];
  
  for(x=0;x<dim_l;x++)
    for(y=0;y<dim_r;y++) 
      if(e_r[y]==-1){
	if(x<dim_l-1){ 
	  if(dir==1) 
	    m[x][y]=ATTC->deletionCostOfEdge(e_l[x],l,r,(Graph*) WModel->owner); 
	  else
	    m[x][y]=ATTC->deletionCostOfEdge(e_l[x],r,l,(Graph*) WModel->owner);
	}
      }else if(e_l[x]==-1){
	if(y<dim_r-1) m[x][y]=ATTC->insertionCostOfEdge(e_r[y],1,1,NULL);
      }else{
	m[x][y]=ATTC->error(e_l[x],e_r[y]);
	if(m[x][y]<0) m[x][y]=INFTY_COST;
      }
  
#if CONTROL_TEST
  int* path;
  path=new int[dim_l];
  ret_val=HS_smart(m,dim_l,dim_r,path);
  int* ed_l,*ed_r;
  ed_l=new int[dim_l];
  ed_r=new int[dim_r];
  collect_edge_edits(dim_l,dim_r,path,ed_l,ed_r,m,SUB_list,DEL_list,INS_list);
#else
  ret_val=HS(m,dim_l,dim_r);
#endif

  for(x=0;x<dim_l;x++) delete m[x];
  delete m;
  return ret_val;
}
#else


double edgeErrorBetween2Vertices(AdjazenzMatrix* WModel, AttributeClass* ATTC, AttId* e_l,int dim_l,int l,int r,int dir){
  int dim_r,edg;
  AttId e_r[1],at;
  
  dim_r=0;
  
  if((l>-1)&&(r>-1)){
    if(dir==1)
      WModel->initNext(l,r);
    else
      WModel->initNext(r,l);
    while((at<WModel->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
      e_r[dim_r++]<at;
    }
  }
  if(dim_r==0)
    e_r[dim_r++].set(-1,NULL,0);
  
  gmt_assert(dim_l<3);
  gmt_assert(dim_r<2);
  
  dim_l=1;
  
  if((e_r[0]==-1)&&(e_l[0]==-1)) return 0;
  
  if(e_r[0]==-1){
    if(dir==1) 
      return ATTC->deletionCostOfEdge(e_l[0],l,r,(Graph*) WModel->owner); 
    else
      return ATTC->deletionCostOfEdge(e_l[0],r,l,(Graph*) WModel->owner);
  }else if(e_l[0]==-1){
    return ATTC->insertionCostOfEdge(e_r[0],1,1,NULL);
  }else{
    double err=ATTC->error(e_l[0],e_r[0]);
    if(err<0) err=INFTY_COST;
    return err;
  }
}


#endif


double Lookahead_result(Token* Original, int p0, int p1, Token* partial, AdjazenzMatrix *model, AdjazenzMatrix *image, int* AVAIL, AttributeClass* ATTC){
  int dir,del;
  int x,y,z,f,dim_l,edg,dummies;
  int st0,st1;
  //AttId e_l[2],
  AttId o_at,i_at,at,e_at;  
  double t_err,err,m_err,max_err,del_cost;
  int nodes;
  int *i,*j;
  int en,en2;

  dummies=0;
  del=0;
  dir=1;
  t_err=0;
  max_err=0;

 
  if(image->Dimension==0) return 0;

  at=image->getNodeAttributeId(0); 
  del_cost=ATTC->deletionCost(at);

  
  st0=0;
  st1=p0;
  
  for(x=p0;x<p1;x++){
    if((*partial)[x-p0]<0){
      dummies++;
      continue;
    }
  }
  
  // future error estimation has quadratic complexity !
  int d;
  if(image->isDirected()) d=2;
  else d=1;
  
  for(f=1;f<2;f++){
      if(f==1){
	  st0=p1;
	  st1=Original->length();
      }
      
      for(y=st0;y<st1;y++){
	  
	  err=INFTY_COST;
	  m_err=INFTY_COST;
	  
	  if(1){
	      o_at=model->getNodeAttributeId((*Original)[y]);
	      
	      for(z=-1;z<image->numberOfVertices();z++){
		  
		  if(z>-1){
		      if(!AVAIL[z]) continue;
		      
		      i_at=image->getNodeAttributeId(z);	  
		      err=ATTC->error(o_at,i_at);
		  }else{
		      err=ATTC->deletionCost(o_at);
		  }
		  
		  if((err<INFTY_COST)&&(!ENV_EDGE_FORWARD_CHECKING_DISABLED)){
		      
		      for(x=p0;x<p1;x++){ 
			  
			  i=&Original->t[x];
			  j=&partial->t[x-p0];
			  if((*j)==-1) continue;
			  
#if 0
			  
			  if((en=model->isEdge((*Original)[x],(*Original)[y]))==-1) continue;
			  
			  for(dir=1;dir>=-1;dir=dir-2){
			      if(dir==1)
				  model->initNext(*i,(*Original)[y]);
			      else{
				  if(image->isDirected()==0) continue;
				  model->initNext((*Original)[y],*i);
			      }
			      
			      dim_l=0;
			      while((at<model->isNextEdgeAttId(&edg))!=NO_ATTRIBUTE){
				  e_l[dim_l++]<at;
			      }
			      
			      e_l[dim_l++].set(-1,NULL,0);
			      err+=edgeErrorBetween2Vertices(image,ATTC,e_l,dim_l,*j,z,dir);
			      
			  }
#else
			  
			  
			  
			  
			  
#ifdef SUPERNODES
			  AttId at;	      
			  if(z>-1){	    
			      image->initNext(*j,z);
			      
			      at<image->isNextEdgeAttId(&edg);
			  }else{
			      at=NO_ATTRIBUTE;
			  }
			  if(at==NO_ATTRIBUTE) en2=-1;
			  
			  
			  model->initNext((*Original)[x],(*Original)[y]);
			  e_at=model->isNextEdgeAttId(&edg);
			  
			  if(e_at==NO_ATTRIBUTE){
			      if(at==NO_ATTRIBUTE) continue;
			      err+=ATTC->insertionCostOfEdge(at,1,1,NULL);
			      continue;
			  }
			  
			  if(at==NO_ATTRIBUTE)
			      //err+=ATTC->deletionCostOfEdge(e_at,(*Original)[x],(*Original)[y],(Graph*) model->owner);
			      err+=ATTC->deletionCostOfEdge(e_at,*j,z,(Graph*) image->owner);
			  else
			      err+=ATTC->error(e_at,at);
			  
			  
#else
			  if(z>-1)    
			      en2=image->isEdge(*j,z);
			  else
			      en2=-1;
			  
			  if((en=model->isEdge((*Original)[x],(*Original)[y]))==-1){
			      if(en2==-1) continue;
			      err+=ATTC->insertionCostOfEdge(image->EdgeAttributes[en2].edgeAtt,1,1,NULL);
			      
			      continue;
			  }
			  
			  e_at=model->EdgeAttributes[en].edgeAtt;
			  if(en2==-1) {
			      err+=ATTC->deletionCostOfEdge(e_at,(*Original)[x],(*Original)[y],(Graph*) model->owner); 
			  }else{
			      
			      err+=ATTC->error(e_at,image->EdgeAttributes[en2].edgeAtt);
			      
			  }
			  
#endif
#endif
			  
			  
		      }
		  }
		  
		  if(m_err>err) m_err=err;
		  if(m_err==0) break;
		  
	      }
	      if(m_err>=INFTY_COST){
		  del++;
		  t_err+=del_cost;
	      }else{
		  if(max_err<m_err) max_err=m_err;
		  t_err+=m_err;
	      }
	  }
	  
	  
      }
  }
  
  nodes=image->numberOfVertices()+dummies;
  nodes=model->numberOfVertices()-nodes;
  nodes-=del;
  if(nodes>0){
    t_err+=nodes*(del_cost-max_err);
  }
  return t_err;
  
}  





void truncateName(char* name){
  char *temp,*p;
  if(name==NULL) return;
  int s=strlen(name);
  temp=new char[s];
  strcpy(temp,name);
  p=strrchr(temp,(int) '/');
  if(p==NULL) return;

  strcpy(name,&p[1]);

}



#ifdef USE_ULLMAN

#include "Matcher.h"



Matcher T;
ModelDataBaseType MD[1];

int depthFirstTreeSearch(AdjazenzMatrix* Model,AdjazenzMatrix *Image,INT_TYPE* vertices,int* images,int start_level,List& Instances){
    int x;
    int pset[7]={0,2,1,2,1,1,1};
    T.setParameters((int*) pset);
	
    MD[0].G=Model->owner;
    MD[0].key=0;

    T.initAttributes(&ATT_object);
    T.setModelDataBase(MD,1);

    T.match(Image,-1);

    Token t;
    int* seq;
    MappingData *mp;
    int ret=T.NumberOfMatches();
    for(x=0;x<ret;x++){
	mp=T.query(x);	
	Instances.insert(mp);
    }

    T.Collection.clear();
    return ret;

}


#else



int depthFirstTreeSearch(AdjazenzMatrix* Model,AdjazenzMatrix *Image,INT_TYPE* vertices,int* images,int start_level,List& Instances){

    int d,level,x,y;
    int dim=Model->numberOfVertices();
    int dim_i=Image->numberOfVertices();
    int en1,en2;
    AttId mat,iat;

    if(start_level==Model->numberOfVertices()) return 1;
    
    double err;
    int good;

    int** FAT;
    int* current;

    FAT=new int*[dim-start_level];
    current=new int[dim-start_level];
    for(x=0;x<dim-start_level;x++){
	FAT[x]=new int[dim_i-start_level];
	mat=Model->NodeAttributes[vertices[x+start_level]];
	for(y=0;y<dim_i-start_level;y++){
	    iat=Image->NodeAttributes[images[y+start_level]];

	    err=ATT_object.error(mat,iat);
	    
	    if(err==0)
		FAT[x][y]=1;
	    else
		FAT[x][y]=0;
	}
    }

    
    d=start_level;
    current[0]=-1;
    Hash used(10);

    while(d>=start_level){
	
	good=0;
	for(x=current[d-start_level]+1;x<dim_i-start_level;x++)
	    if((FAT[d-start_level][x])&&(!used.in(x))){
		used.remove(current[d-start_level]);
		current[d-start_level]=x;
		used.insert(x);
		good=1;
		break;
	    }
	
	if(good==0){
	    used.remove(current[d-start_level]);
	    current[d-start_level]=-1;
	    d--;
	    continue;
	}
	
	for(x=0;x<d;x++){
	    en1=Model->isEdge(vertices[x],vertices[d]);
	    if(x<start_level)
		en2=Image->isEdge(images[x],images[current[d-start_level]+start_level]);
	    else
		en2=Image->isEdge(images[current[x-start_level]+start_level],images[current[d-start_level]+start_level]);

	    if(en1==-1){
		if(en2!=-1)
		    good=0;
	    }else if(en2==-1){
		good=0;
	    }else{
		mat=Model->EdgeAttributes[en1].edgeAtt;
		iat=Image->EdgeAttributes[en2].edgeAtt;

		err=ATT_object.error(mat,iat);
		if(err>0)
		    good=0;
	    }

	    if(good==0)
		break;
	}
	

	if(good){
	    d++;
	    
	    if(d==dim){
		int* seq=new int[dim-start_level];
		for(x=0;x<dim-start_level;x++)
		    seq[x]=images[current[x]+start_level];
		
		Instances.insert(seq);

		d--;
	    }else{
		current[d-start_level]=-1;
	    }
	}

    }


    delete [] current;
    for(x=0;x<dim-start_level;x++)
	delete [] FAT[x];
    delete [] FAT;

    return Instances.count();

}




#endif



