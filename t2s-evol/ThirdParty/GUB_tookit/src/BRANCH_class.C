#include "BRANCH_class.h"

int BRANCH_class::size=0;
int BRANCH_class::checks=0;
Graph** BRANCH_class::MODELS_IN_TREE=NULL;

BRANCH_class::BRANCH_class(){
  root=NULL;
  checks=0;
}


BRANCH_class::~BRANCH_class(){

    deleteRec(root);


}


void
BRANCH_class::deleteRec(BIN_class* node){
    BIN_class* next;
    EDGE_nextclass* en,*en2;


    if(node==NULL) return;
    
    if(node->successors!=NULL){
	node->successors->clearDeep();
	delete node->successors;
    }
    
    next=(BIN_class*) node->succ[0];
    if(next)
	deleteRec(next);
    en=(EDGE_nextclass*) node->succ[1];
    while(en!=NULL){
	if(en->son)
	    deleteRec(en->son);
	en2=en;
	en=en->next;
	delete en2;
    }
    delete node;
}


int 
BRANCH_class::test(int* edges,int vertex,AdjazenzMatrix* AM){
int i;
BIN_class* s;
int v1,v2;
AttId *e_at;

   i=0;
   if(root!=NULL){
     s=root;
     while(i<depth){
#if 0       
       s=s->succ[edges[i]];
       
#else
       if(edges[i]>-1)
	 e_at=&AM->EdgeAttributes[edges[i]].edgeAtt;
       s=testEdge(e_at,edges[i],s->succ);
#endif
       if(s==NULL) return -1;
       i++;
       checks++;
     }

     AttId *v_at=&AM->NodeAttributes[vertex];
     Next_class *np;

     if(s->successors!=NULL){
       AttId *at;
       s->successors->reset();
       while(np=(Next_class*) s->successors->getnext()){
	 at=&MODELS_IN_TREE[np->model]->AM.NodeAttributes[np->vertex];
	 if(ATT_object.error(*at,*v_at)==0)
	   return np->next;
       }
     }
     return -1;
   }else{
     return -1;
   }
}


int 
BRANCH_class::add(int* edges,int node,int step,int vertex, AdjazenzMatrix* AM,int num){
    int i,v1,v2;
    BIN_class* s,*s2,*s1;
    AttId *e_at;


/*
    root(BIN_class)
       \
        [0,1],successors (list of vertex attributes)
        /   \
BIN_class     EDGE_nextclass                            ->next      ->next ->0
  [0,1]           BIN_class,model,egde (edge attribute)
  /   \             /
BIN_class          [0,1]
    
*/


     
    i=0;
    if(root==NULL){
	root=new BIN_class;
	root->succ[0]=NULL;
	root->succ[1]=NULL;
	root->successors=NULL;
	size++;
    }
    
   
    depth=step;
    s=root;
    while(i<depth){
      if(edges[i]>-1)
	e_at=&AM->EdgeAttributes[edges[i]].edgeAtt;
      
      
      
#if 0
	if(s->succ[edges[i]]==NULL){
	  s->succ[edges[i]]=new BIN_class;
	  s->succ[edges[i]]->succ[0]=NULL;
	  s->succ[edges[i]]->succ[1]=NULL;
	  s->succ[edges[i]]->successors=NULL;
	  size++;   
	}
	s=s->succ[edges[i]];
#else
      if((s1=testEdge(e_at,edges[i],s->succ))==NULL){
	if(edges[i]+1==0){
	  s2=new BIN_class;
	  s2->succ[0]=NULL;
	  s2->succ[1]=NULL;
	  s2->successors=NULL;
	  s->succ[0]=s2;
	}else{
	  EDGE_nextclass* es=(EDGE_nextclass*) s->succ[1];
	  EDGE_nextclass* en;
	  

	  en=new EDGE_nextclass;
	  s2=new BIN_class;
	  s2->succ[0]=NULL;
	  s2->succ[1]=NULL;
	  s2->successors=NULL;
	  en->son=s2;
	  en->model=num;
	  en->edge=edges[i];
	  en->next=NULL;

	  if(es!=NULL){
	    while(es->next!=NULL)
	      es=es->next;
	    es->next=en;
	  }else{
	    s->succ[1]=en;
	  }
	}
	s=s2;

#endif
	
      }else{
	s=s1;
      }
      i++;
    }

  Next_class *np;

  if(s->successors==NULL)
       s->successors=new List;
       
   np=new Next_class;
   np->model=num;
   np->vertex=vertex;
   np->next=node;
   s->successors->insert(np);

  return np->next;
    
};


BIN_class*
BRANCH_class::testEdge(AttId* at,int edge,void** succ){
   EDGE_nextclass* en;
   int v1,v2;

   if(edge==-1){
     return (BIN_class*) succ[0];
   }else{
     AttId *eat;
     en=(EDGE_nextclass*) succ[1];
     while(en!=NULL){
       eat=&MODELS_IN_TREE[en->model]->AM.EdgeAttributes[en->edge].edgeAtt;
       if(ATT_object.error(*eat,*at)==0){
	 return en->son;
       }
       en=en->next;
     }
     return NULL;
   }
}


int
BRANCH_class::write(FILE* file){

    fwrite((char*) &depth,sizeof(int),1,file);
    writeNode(root,file);


    return 1;
}


int
BRANCH_class::writeNode(BIN_class* node,FILE* file){
    int flag;
    int sf=sizeof(int);
    
    if(node==NULL){
	flag=0;
	fwrite((char*) &flag,sf,1,file);  
	return 0;
    }

    flag=1;
    fwrite((char*) &flag,sf,1,file);

    if(node->successors==NULL){
	flag=0;
	fwrite((char*) &flag,sf,1,file);
    }else{
	Next_class* nc;
	flag=node->successors->count();
	fwrite((char*) &flag,sf,1,file);
	node->successors->reset();
	while(nc=(Next_class*) node->successors->getnext()){
	    fwrite((char*) nc,sizeof(Next_class),1, file);
	}
    }
    
    
    writeNode((BIN_class*) node->succ[0],file);

    if(node->succ[1]){
	EDGE_nextclass* en;
	flag=1;
	en=(EDGE_nextclass*) node->succ[1];
	while(en){
	    fwrite((char*) &flag,sf,1,file);
	    fwrite((char*) &en->model,sizeof(int),1,file);
	    fwrite((char*) &en->edge,sizeof(INT_TYPE),1,file);
	    writeNode(en->son,file);
	    en=en->next;
	}
	
	
    }
    flag=0;
    fwrite((char*) &flag,sf,1,file);

    return 1;
}


int
BRANCH_class::read(FILE* file){

    fread((char*) &depth,sizeof(int),1,file);
    root=readNode(file);
    return 1;
}



BIN_class*
BRANCH_class::readNode(FILE* file){
    int flag;
    int sf=sizeof(int);
    BIN_class* node;
	
    fread((char*) &flag,sf,1,file);  
    if(!flag) return NULL;

    node=new BIN_class;
    node->succ[0]=NULL;
    node->succ[1]=NULL;
    node->successors=NULL;
    
    fread((char*) &flag,sf,1,file);

    if(flag){
	int c;
	Next_class* nc;
	node->successors=new List;
	for(int x=0;x<flag;x++){
	    nc=new Next_class;
	    fread((char*) nc,sizeof(Next_class),1,file);
	    node->successors->insert(nc);
	}
    }

    

    node->succ[0]=(void*) readNode(file);
        
    fread((char*) &flag,sf,1,file);
    
    EDGE_nextclass* top=NULL;
    EDGE_nextclass* en=NULL;
    while(flag){
	
	if(en==NULL){
	    en=new EDGE_nextclass;
	    top=en;
	    en->next=NULL;
	}else{
	    en->next=new EDGE_nextclass;
	    en=en->next;
	    en->next=NULL;
	}

	fread((char*) &en->model,sizeof(int),1,file);
	fread((char*) &en->edge,sizeof(INT_TYPE),1,file);
	en->son=readNode(file);
	
	fread((char*) &flag,sf,1,file);
    }

    node->succ[1]=(void*) top;

    return node;

}
