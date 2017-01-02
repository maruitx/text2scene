#include "gmt_assert.h"


void gmt_assert(int C){
int a;

  if(C!=0) return;

  cout << "ASSERT FAILED!! (type to exit) .... ";
  cin  >> a;



//  exit(10);

}

