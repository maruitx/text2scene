#ifndef _learn
#define _learn

#include"List.h"
#include"Graph.h"
#include"GMClass.h"




double** distanceBetweenGraphs(List *NewModels,double threshold);

void clustering(double** m,List *NewModels, List *CL,double threshold);

List* learningCircuits(Graph* G,double threshold);


#endif
