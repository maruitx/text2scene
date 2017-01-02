#include "initrandomize.h"

static int FLAG = 0;



void initzufall(){

  int itime;
  long ltime;

  ltime=time(0);

  itime=(unsigned int) ltime/2;

  if(FLAG==0)
    srand(itime);

  FLAG=1;
};

 
