/*
Tests for Da.

*/

#include <stdbool.h>
#include <da.h>

#include "test_da.lib.h"


int test_da_0(){
  Da da;
  da_alloc(&da, sizeof(int)); // leaves room for 4 ints
  int i = 0;
  int *pt = (int *)da.base;
  // will double, 4 -> 8, then double 8 -> 16
  while( i < 10 ){
    da_push(&da, &i);
  i++;
  pt++;
  }

  bool f0 = da.size == sizeof(int) * 16;
  bool f1 = 10 == (da.end - da.base) / sizeof(int);
  bool f2 = true;
  pt = (int *)da.base;
  i = 0;
  while( i < 10 ){
    f2 = f2 && *pt == i && !da_endq(&da, pt);
  i++;
  pt++;
  }
  bool f3 = da_endq(&da, pt);

  return f0 && f1 && f2 && f3;
}

int test_da_1(){
  Da da;
  da_alloc(&da, sizeof(int)); // leaves room for 4 ints
  int i = 0;
  int *pt = (int *)da.base;
  // will double, 4 -> 8, then double 8 -> 16
  while( i < 10 ){
    da.end += da.item_size;
    if( da_boundq(&da) ){
      char *old_base = da_expand(&da);
      da_rebase(&da, old_base, &pt);
    }
    *pt = i;
  i++;
  pt++;
  }

  bool f0 = da.size == sizeof(int) * 16;
  bool f1 = 10 == (da.end - da.base) / sizeof(int);
  bool f2 = true;
  pt = (int *)da.base;
  i = 0;
  while( i < 10 ){
    f2 = f2 && *pt == i && !da_endq(&da, pt);
  i++;
  pt++;
  }
  bool f3 = da_endq(&da, pt);

  return f0 && f1 && f2 && f3;
}


