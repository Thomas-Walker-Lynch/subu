/*
Tests for da.

*/

#include "da.h"
#include "test_da.lib.h"
#include <stdbool.h>

int test_da_0(){
  da da0;
  da_alloc(&da0, sizeof(int)); // leaves room for 4 ints
  int i = 0;
  int *pt = da0->base;
  // will double, 4 -> 8, then double 8 -> 16
  while( i < 10 ){
    if(da_boundq(&da0, pt)){
      char *old_base = da_expand(&da);
      da_rebase(&da, old_base, pt);
    }
    *pt = i;
  i++;
  pt++;
  }

  bool f0 = da.size == sizof(int) * 16;
  bool f1 = 10 == (da.end - da.base) / sizeof(int);
  bool f2 = true;
  pt = da0->base;
  while( i < 10 ){
    f2 = f2 && *pt == i && !da_endq(&da, pt);
  i++;
  pt++;
  }
  bool f3 = da_endq(&da, pt);

  return f0 && f1 && f2 && f3;
}


