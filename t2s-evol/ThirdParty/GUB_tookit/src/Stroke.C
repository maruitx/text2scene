#include "Stroke.h"



void 
line::set(double x1,double y1,double x2,double y2){

  l.x=x1;
  l.y=y1;

  r.x=x2;
  r.y=y2;

  if(fabs(r.x-l.x)<0.01) VERTICAL=1;
  else VERTICAL=0;

  if(!VERTICAL){   
    Degree=(r.y-l.y)/(r.x-l.x);
    trans=y2-Degree*x2;
  }

  Length=sqrt((r.y-l.y)*(r.y-l.y)+(r.x-l.x)*(r.x-l.x));
    
}

void
line::setKey(int k){
  Key=k;
}

int 
line::key(){
  return Key;
}


point 
line::middle(){
  point p;

  p.x=(r.x + l.x) / 2;
  p.y=(r.y + l.y) / 2;

  return p;

}


double 
line::angle(line* l1){
  double sig;

  return angle(l1,sig);
}


double 
line::angle(line* l1,double &sig){
  double a1,a2,d1,d2,fl;

/* determine where l1 and (this) are hooked together */

  if(((l.x==l1->l.x)&&(l.y==l1->l.y)&&(r.x==l1->r.x)&&(r.y==l1->r.y))||
     ((l.x==l1->r.x)&&(l.y==l1->r.y)&&(r.x==l1->l.x)&&(r.y==l1->l.y))){
    a1=0;
    a2=0;
  }else{


    if(((l.x==l1->l.x)&&(l.y==l1->l.y))||((l.x==l1->r.x)&&(l.y==l1->r.y))){
      if(r.x==l.x){
	if(l.y<r.y) a1=PI/2;
	else a1=PI*3/2;
      }else{
	a1=atan2(r.y-l.y,r.x-l.x);
	if(a1<0) a1=2*PI+a1;
      }
    }else{
      if(r.x==l.x){
	if(l.y>r.y) a1=PI/2;
	else a1=PI*3/2;
      }else{
	a1=atan2(l.y-r.y,l.x-r.x);
	if(a1<0) a1=2*PI+a1;
      }
    }
    
    if(((l1->l.x==l.x)&&(l1->l.y==l.y))||((l1->l.x==r.x)&&(l1->l.y==r.y))){
      if(l1->r.x==l1->l.x){
	if(l1->l.y<l1->r.y) a2=PI/2;
	else a2=PI*3/2;
      }else{
	a2=atan2(l1->r.y-l1->l.y,l1->r.x-l1->l.x);
	if(a2<0) a2=2*PI+a2;
      }
    }else{
      if(l1->r.x==l1->l.x){
	if(l1->l.y>l1->r.y) a2=PI/2;
	else a2=PI*3/2;
      }else{
	a2=atan2(l1->l.y-l1->r.y,l1->l.x-l1->r.x);
	if(a2<0) a2=2*PI+a2;
      }
    }
  }
  // calculate direction of angle: projection of l1 onto the transformed 
  // coordinate system with origin in l. 
  

  d2=PI/2-(a2-a1);
  fl=cos(d2);
  if(fl<0) sig=-1;
  else sig=1;

  /* return the small enclosed angel between l1 and (this) , make it positive! */
  d1=fabs(a1-a2);
  if(d1<0) d1=-d1;
  if(d1>PI) d1=2*PI-d1;
  return d1;


}


int 
line::incidencePoint(line* l1){
  if(((l.x==l1->l.x)&&(l.y==l1->l.y))||((l.x==l1->r.x)&&(l.y==l1->r.y))){
    return 0;
  }else{
    return 1;
  }
}
  

int
line::intersect(line& lin,double threshold){
  double x,y;
  point p;

  

  if(VERTICAL){
      if(lin.VERTICAL) {
	  x=(l.x+lin.l.x)/2;
	  y=threshold/0.001;
      }else{         
	  x=l.x;
	  y=lin.Degree*x+lin.trans;
      }
  }else{
      if(lin.VERTICAL)
	  x=lin.l.x;
      else{
	  if(fabs(lin.Degree-Degree)<0.001){
	      if(lin.Degree>Degree)
		  x=(trans-lin.trans)/0.001;
	      else
		  x=-(trans-lin.trans)/0.001;
	  }else{
	      x=(trans-lin.trans)/(lin.Degree-Degree);
	  }      
      }  
      y=Degree*x+trans;
  }

  
  CP.x=x;
  CP.y=y;
  
  
  if(VERTICAL){
      if(lin.VERTICAL) return 0;
  }else{
      if(!lin.VERTICAL){  
	  if(fabs(Degree-lin.Degree)<0.001) return 0;
      }
  }
  
  if(lin.in(CP,threshold)&&in(CP,threshold)) return 1;
  else return 0;
  
}
  

double 
line::distance(line &lin){
  double x,y,r1,r2;
  point p;

  intersect(lin,0);
  p=CP;
  
#if 1
  r1=sqrt((p.y-l.y)*(p.y-l.y)+(p.x-l.x)*(p.x-l.x));
  r2=sqrt((p.y-r.y)*(p.y-r.y)+(p.x-r.x)*(p.x-r.x));

  if(!lin.intersect(*this,0)) {
    r1=(-1)*r1;
    r2=(-1)*r2;
  }
  

  if(fabs(r1)<fabs(r2)) return r1;
  else return r2;

  
#endif
}
  



double 
line::length(){

  return Length;
}
  


double 
line::degree(){
  double d;


  if(VERTICAL){
    if(l.y<r.y) return PI/2;
    else return PI*3/2;
  }
#if 0
 
  d=atan(Degree);
  if(Degree>=0){
    if(l.x>r.x)
      d=d+PI;
  }else{
    if(l.x<r.x)
      d=d*(-1);
    else
      d=PI+d;
  }
#else
  d=atan2(r.y-l.y,r.x-l.x);

  if(d<0) d=-d;
  if(d>2*PI) d=d-2*PI;

#endif
  return d;
}
  

double 
line::normDegree(){
  double d;

  if(VERTICAL) return PI/2;

  d=atan(Degree);
  if(d<0) d=PI+d;
  return d;
}


int
line::in(point p,double threshold){
  int xgood,ygood;
  line dl;
  point pm;

#if 0
  xgood=0;
  ygood=0;

  if(l.x>r.x){
    if((p.x>=r.x)&&(p.x<=l.x)) xgood=1;
  }else{
    if((p.x<=r.x)&&(p.x>=l.x)) xgood=1;
  }

   if(l.y>r.y){
    if((p.y>=r.y)&&(p.y<=l.y)) ygood=1;
  }else{
    if((p.y<=r.y)&&(p.y>=l.y)) ygood=1;
  }

  if((xgood)&&(ygood)) return 1;
  
  return 0;
#else

  pm=middle();
  dl.set(pm.x,pm.y,p.x,p.y);
  if(dl.length()>(Length/2 + threshold)) return 0;
  else return 1;

  

#endif
}



void
Stroke::readEpicFile(char*name,AttributeClass *ATT){
  char what;
  char out[256];
  int x,x1,x2,y1,y2;
  FILE* file;

  file=fopen(name,"r");

  lineDim=0;

  what=getc_c(file);

  while(!feof(file)){
   
    switch(what){
      
    case '\\': break;
    case 'p': 
      for(x=0;x<4;x++) what=getc_c(file);
      x1=(int) get_zahl(file);
      y1=(int) get_zahl(file);
      what=getc_c(file);
      x2=(int) get_zahl(file);
      y2=(int) get_zahl(file);
      
      LINES[lineDim].set(x1,y1,x2,y2);
      lineDim++;
      break;
    case '\n': break;
    default: fgets(out,255,file);
    }	    
    what=getc_c(file);
  }

  transformLinesIntoGraph(ATT);

}




void  
Stroke::transformLinesIntoGraph(AttributeClass* ATT){
  int x,y;
  double d;
  point p1,p2;
  line arc;
  int edges;
  double val[5];
  AttId at;

  create(lineDim,1);
  
  for(x=0;x<lineDim;x++){
    val[0]=LINES[x].normDegree();
    val[1]=LINES[x].l.x;
    val[2]=LINES[x].l.y;
    val[3]=LINES[x].r.x;
    val[4]=LINES[x].r.y;
    at=ATT->registerLabel(1,val,5);
    setNodeAttributeId(x,at);
  
  }

  edges=0;
  for(x=0;x<lineDim;x++){
    for(y=0;y<lineDim;y++){
      if(x==y) continue;

      if(LINES[x].intersect(LINES[y],0)){
	d=LINES[x].distance(LINES[y]);
	val[1]=d/LINES[x].length();

	if(fabs(val[1])>3) continue;
	p1=LINES[x].middle();
	p2=LINES[y].middle();
	arc.set(p1.x,p1.y,p2.x,p2.y);
	val[0]=arc.degree();
	at=ATT->registerLabel(2,val,2);
	setEdgeAttributeId(edges,at,x,y);
	setEdge(x,y,edges);
	edges++;
      }
    }
  }
}


void
Stroke::writeToFile(char* name,int num,AttributeClass *ATT,FILE* file){

  AdjazenzMatrix::COMPLEX_ATTRIBUTES=1;

  AdjazenzMatrix::writeToFile(name,num,ATT,file);

  AdjazenzMatrix::COMPLEX_ATTRIBUTES=0;

}
