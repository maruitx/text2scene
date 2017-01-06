#include "Randomizer.h"

int Maximum_Int = 100000;
long FIRST_TIME = 0;


double rando()
{

  int itime,x;
  long tim;
  double max,sum,ret,z;
  max = 100;

  tim = time(NULL);
/*
  if(tim-FIRST_TIME>20){
    FIRST_TIME=tim;
    itime = (unsigned int) tim/2;
    srand(itime);
  }
 */
  
  sum = 0;
  for(x=0;x<ITER;x++){
    
    if((z = zufall())>Maximum_Int) Maximum_Int=(int) z+2;
    z=z/((double) Maximum_Int);
    z = z*max;
    sum = sum + z;
  }

  sum = sum/ITER;
  sum = sum - max/2;

  ret = sum/max;

  return ret;
}
