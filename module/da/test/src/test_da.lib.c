/*
Tests for Da.

*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <da.h>

#include "test_da.lib.h"

// tests push
bool test_da_push_0(){
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
bool test_da_expand_0(){
  Da da;
  da_alloc(&da, sizeof(int)); // leaves room for 4 ints
  int i = 0;
  int *pt = (int *)da.base;
  // will double, 4 -> 8, then double 8 -> 16
  while( i < 10 ){
    da.end += da.element_size;
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

// da_fgets via da_string_input
bool test_da_string_input_0(){

  char *data_file_name = "../lib/test.dat";
  FILE *file = fopen(data_file_name,"r");
  if(!file){
    fprintf(stderr,"test_da_2, could not open data file %s for reading\n", data_file_name);
    return false;
  }

  Da da;
  da_alloc(&da, sizeof(char));

  da_string_input(&da, file);
  bool f0 = !strcmp(da.base, "this is a test");

  char *old_base;
  da_pop(&da, NULL); // pop the prior null terminator
  char *s1 = da.end;
  old_base = da_string_input(&da,file);
  da_rebase(&da, old_base, &s1);
  bool f1 = !strcmp(s1, "ends without a newline");
  
  da_pop(&da, NULL); // pop the prior null terminator
  char *s2 = da.end;
  old_base = da_string_input(&da,file);
  da_rebase(&da, old_base, &s2);
  bool f2 = !strcmp(s2, "(setq mode-require-final-newline nil)");

  bool f3 = !strcmp(da.base, "this is a testends without a newline(setq mode-require-final-newline nil)");

  fclose(file);
  return f0 && f1 && f2 && f3;
}

// da_pop
bool test_da_pop_0(){

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

// da_cat
bool test_da_cat_0(){

  Da da0, da1;
  da_alloc(&da0, sizeof(int));
  da_alloc(&da1, sizeof(int));

  int i = 5;
  while(i < 8){
    da_push(&da0, &i);
  i++;
  }
  while(i < 11){
    da_push(&da1, &i);
  i++;
  }

  da_cat(&da0, &da1);

  bool f[6];
  int j;
  int k = 0;
  while(k < 6){
    f[k] = da_pop(&da0, &j) && (j == 10 - k);
  k++;
  }

  bool result = f[0];
  k = 1;
  while(result && k < 6){
    result = f[k];
  k++;
  }

  return result;
}

//Glenda: this was already tested much better above, where each element was actually tested, not just one, but it was a good one to test for practice
bool test_da_cat_1(){
  Da dar0;
  Da dar1;
  da_alloc(&dar0, sizeof(int));
  da_alloc(&dar1, sizeof(int));
  int a = 2;
  for (int b=0; b<4; b++){
    da_push(&dar0, &a);
    a+=2;
  }
  size_t off0 = dar0.end - dar0.base;
  bool f1 = dar0.base != dar0.end;
  for (int b=0; b<4; b++){
    da_push(&dar1, &a);
    a+=2;
  }
  size_t off1 = dar1.end - dar1.base;
  bool f2 = dar1.base != dar1.end;
  da_cat(&dar0, &dar1);
  bool f3 = dar0.end == dar0.base + off0 +off1;
  bool f4 = *(dar0.base + (5*dar0.element_size)) == *(dar1.base + dar1.element_size);
  return f1 && f2 && f3 && f4;
}


//Glenda's tests
//tested output for success and artificial failure

//tests da_rewind
bool test_da_rewind_0(){
  int i = 10;
  Da dar;
  da_alloc(&dar, sizeof(int));
  while(i < 18){
    da_push(&dar, &i);
    ++i;
  }
  bool f1 = dar.end != dar.base;
  da_rewind(&dar);
  int j = *(dar.end);
  bool f2 = dar.end == dar.base;
  bool f3 = j == 10;
  //  bool f4 = *dar.end == 10 && *(dar.end + 9 * sizeof(int)) = 19;
  bool result = f1 && f2 && f3; // && f4;
  return result;
}

//tests da_index
bool test_da_index_0(){
  Da dar;
  da_alloc(&dar, sizeof(int));
  int j = 0;
  char *k[2];
  while(j < 4){
    da_push(&dar, &j);
    j++;
  }
  j--;
  k[0] = da_index(&dar, j);
  bool f1 = k[0] == dar.base + (3*dar.element_size);
  return f1;
}

//tests da_free_elements
//cannot test sub-functions bc da_free_elements needs to be static
/*bool test_da_7(){
    Da dar;
    da_alloc(&dar, sizeof(int));
    int j = 0;
    while(j < 4){
      da_push(&dar, &j);
      ++j;
    }
    da_free_elements(&dar);
    bool g = 1;
    return g;
  }
causes errors: 
test_da_2, could not open data file ../lib/test.dat for reading
test_da_2 failed - these were because in wrong directory,  core dump is actual issue here and with 7
Segmentation fault (core dumped)
*/
/*
//tests da_strings_exist
bool test_da_7(){
  Da dar;
  da_alloc(&dar, sizeof(char *));
  char *j = "test";
  da_push(&dar, j);
  //  da_strings_exists(&dar, j);
  bool f1 =  da_strings_exists(&dar, j);
  return f1;
}
//same errors, something to do with calling fucntions that call static functions I believe
*/

//tests rebase 
bool test_da_rebase_0(){
  Da dar;
  da_alloc(&dar,sizeof(char));

  char **el_pt = (char **)malloc(4*(sizeof(char*)));
  {//push 2-5 into dar, leave with pointer to last element
    int i = 0;
    char arr[4] = {'t','e','s','t'};
    while(i<4){
      char c = arr[i];
      *el_pt = da_push(&dar, &c);
      el_pt++;
      i++;
    }
    el_pt--;
  }

  //check that push worked, that element pointer is in right place, and that expand works
  //
  bool f1 = dar.base != dar.end;
  bool f2 = *el_pt == dar.end - dar.element_size;

  char *obase = dar.base;
  char *old_base = da_expand(&dar);
  bool f3 = obase == old_base;

  bool f4[4];
  {//check that each pointer is properly rebased
    int i = 0;
    while(i<4 && *el_pt >= old_base){
      size_t old_offset = *el_pt - old_base;
      da_rebase(&dar, old_base, el_pt);
      size_t new_offset = *el_pt - dar.base;
      f4[i] = old_offset == new_offset;
      el_pt--;
      i++;
    }
  }
  
  bool result = f4[0];
  {//now check that all pointers are properly rebased
    int i = 1;
    while(result && i < 4){
      result = f4[i];
      i++;
    }
  }
  
  return f1 && f2 && f3 && result;
}


//tests da_boundq
bool test_da_boundq_0(){
  Da dar;
  da_alloc(&dar,sizeof(char));

  bool f[5];
  {//pushes onto dar and tests no runoff
    char arr[4] = {'r','e','s','t'};
    int i = 0;
    char c;
    while(i<4){
      c = arr[i];
      da_push(&dar, &c);
      f[i] = (dar.end != dar.base) && !da_boundq(&dar);
      i++;
    }
  }

  //makes end pointer run off allocated area, tests boundq detects runoff
  dar.end++; // = dar.base + dar.size;
  f[4] = da_boundq(&dar);

  bool result = true;
  {//check all test results
    int i = 0;
    while (result && i < 5){
      result = f[i];
      i++;
    }
  }
  
  return result;
}

//tests map
bool test_da_map_0(){
  Da dar;
  da_alloc(&dar,sizeof(char));

  {//pushes onto dar
    char arr[4] = {'t','r','u','e'};
    int i = 0;
    char c;
    while(i<4){
      c = arr[i];
      da_push(&dar, &c);
      i++;
    }
  }

  bool *closure;
  bool result = true;
  {//tests map via test_map
    int i = 0;
    while (result && i<4){
      da_map(&dar, test_map, closure);
      result = *closure;
      i++;
      *closure = false;
    }
  }
  
  return result;
}
  
  

/*
  Functions               
da_alloc                    
da_free                     
-da_rewind                 
da_emptyq                   
da_length                   
-da_rebase                   
-da_expand                 
-da_boundq                   
-da_index                  
da_push_alloc
-da_push                   
-da_pop                    
da_endq                     
-da_map           Had to put test_map function in da src code...
da_free_elements            
da_ints_print               
da_strings_print            
da_strings_exists           
da_strings_set_insert       
da_strings_set_union        
-da_string_input           
da_string_push              
-da_cat                    

test first
map
boundq

add
da_exists (OR map, ∃)
da_all (AND map, ∀)

*/
