#ifndef _stroke
#define _stroke
#include <math.h>
//#include <cmath>
#include "AdjazenzMatrix.h"
//#include "AttributeClass.h"
#include "AttControl.h"

using namespace std;

struct point{
  double x;
  double y;
};


class line{
 
 public:

  void set(double x1,double y1,double x2,double y2);
  
  void setKey(int k);
  int key();

  point middle();
  int incidencePoint(line* l1);
  int intersect(line& l,double threshold);
  double distance(line &l);
  double length();
  double degree();
  double normDegree();
  int in(point p,double threshold);
  double angle(line* l1);
  double angle(line* l1,double &sig);

  point l,r;
  double Length;
  double Degree;
  double trans;

  point CP;

  int Key;
  int VERTICAL;
};



#define MAX_LINES 100



class Stroke:public AdjazenzMatrix{

 public:
  
  void readEpicFile(char* name,AttributeClass *ATT);
  
  void  transformLinesIntoGraph(AttributeClass *ATT);
  
  void writeToFile(char* name,int num,AttributeClass *ATT,FILE* file);

 private:

  line LINES[MAX_LINES];
  int lineDim;
  

};

#endif
