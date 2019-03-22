/*
Tests for Da.

*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <da.h>

#include "test_da.lib.h"

// tests push
bool test_da_0(){
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

// tests manual expansion
bool test_da_1(){
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

// da_fgets
bool test_da_2(){

  FILE *fd = fopen("test.dat","r");

  Da da;
  da_alloc(&da, sizeof(char));

  da_fgets(&da, fd);
  bool f0 = !strcmp(da.base, "this is a test");

  char *old_base;
  da_pop(&da, NULL); // pop the prior null terminator
  char *s1 = da.end;
  old_base = da_fgets(&da,fd);
  da_rebase(&da, old_base, &s1);
  bool f1 = !strcmp(s1, "ends without a newline");
  
  da_pop(&da, NULL); // pop the prior null terminator
  char *s2 = da.end;
  old_base = da_fgets(&da,fd);
  da_rebase(&da, old_base, &s2);
  bool f2 = !strcmp(s2, "(setq mode-require-final-newline nil)");

  bool f3 = !strcmp(da.base, "this is a testends without a newline(setq mode-require-final-newline nil)");

  fclose(fd);
  return f0 && f1 && f2 && f3;
}

// da_fgets
bool test_da_3(){

  FILE *fd = fopen("test.dat","r");

  Da da;
  da_alloc(&da, sizeof(int));

  int i = 5;
  da_push(&da, &i);
  i++;
  da_push(&da, &i);
  i++;
  da_push(&da, &i);

  int j;
  bool f0 = da_pop(&da, &j) && j == 7;
  bool f1 = da_pop(&da, &j) && j == 6;
  bool f2 = da_pop(&da, NULL);
  bool f3 = !da_pop(&da, &j);

  return f0 && f1 && f2 && f3;
}




