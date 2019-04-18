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
  bool f4 = (*(dar.end + (7 * sizeof(int))) = 17);
  bool result = f1 && f2 && f3 && f4;
  return result;
}

//tests da_index
bool test_da_index_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_alloc(dar_pt, sizeof(int));

  char *k[4];
  bool f[4];
  {//push to dar and test da_index
    size_t i = 0;
    int j = 13;
    int *j_pt = &j;
    while(i < 4){
      da_push(dar_pt, j_pt);
      j++;
      k[i] = da_index(dar_pt, i);
      f[i] = k[i] == dar.base + (i*dar.element_size);
      i++;
    }
  }
  bool result = f[0] && f[1] && f[2] && f[3];
  return result;
}

//tests da_free_elements - still getting core dump on function call
//cannot test sub-functions bc da_free_elements needs to be static
bool test_da_free_elements_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_alloc(dar_pt, sizeof(int*));

  int i = 3;
  int *i_pt = &i;
  while(i < 7){
    da_push(dar_pt, &i_pt);
    ++i;
  }
  
  //da_free_elements(dar_pt);
  bool result = true;
  return result;
}

//tests da_strings_exist
bool test_da_strings_exists_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_alloc(dar_pt, sizeof(char *));

  //fill dar with strings
  char *string0 = "nope";
  char **string0_pt = &string0;
  da_push(dar_pt, string0_pt);
  char *string1 = "okay";
  char **string1_pt = &string1;
  da_push(dar_pt, string1_pt);
  char *string2 = "welp";
  char **string2_pt = &string2;
  da_push(dar_pt, string2_pt);
  char *string3 = "sure";
  char **string3_pt = &string3;
  da_push(dar_pt, string3_pt);

  bool result;
  {//test dar for each string
    result = da_strings_exists(dar_pt, string0);
    if (result == true)
    result = da_strings_exists(dar_pt, string1);
    if (result == true)
    result = da_strings_exists(dar_pt, string2);
    if (result == true)
    result = da_strings_exists(dar_pt, string3);
  }
 
  return result;
}


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

/* {//problems with free and malloc
  char *a = malloc(10);
  strcpy(a, "zsdf");
  Da da;
  Da *da_pt = &da;
  da_alloc(da_pt, sizeof(char *));
  da_push(da_pt, a);
  ...
    free(*(char *)da_index(da_pt, 0));
    da_free(da_pt);
} */

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


void da_test_map(void *pt, void *closure){
  int *n = (int *)closure;
  *n += *(int *)pt;
}

//tests map
bool test_da_map_0(){
  Da dar;
  da_alloc(&dar,sizeof(int));

  {//pushes onto dar
    int arr[4] = {5,6,7,8};
    int i = 0;
    int n;
    while(i<4){
      n = arr[i];
      da_push(&dar, &n);
      i++;
    }
  }

  int n = 0;
  int *closure = &n;
  
  da_map(&dar, da_test_map, (int *)closure);
  //rename to da_foreach
  bool result = n == (5+6+7+8);
  
  return result;
}

//tests da_exists
//exists is a map function that applies a function to each element of the array and returns a bool
//if the function ever returns a true value, then exists immediately returns true, if the function only ever returns false, then exists returns false
//da_all is the opposite, returns true only if always receives true, exists and returns false if false

//tests da_present
bool test_da_present_0(){
  int dar_size = 0;
  Da **dar = malloc(3 * sizeof(Da *));

  Da dap_0;
  Da *dap_0_pt = &dap_0;
  da_alloc(dap_0_pt,sizeof(int));

  Da dap_1;
  Da *dap_1_pt = &dap_1;
  da_alloc(dap_1_pt,sizeof(int));
  
  dar[dar_size] = dap_0_pt; dar_size++;
  dar[dar_size] = dap_1_pt; dar_size++;
  Da dap_2;
  Da *dap_2_pt = &dap_2;
  da_alloc(dap_2_pt,sizeof(int));
  dar[dar_size] = dap_2_pt; dar_size++;  

  typedef struct{
  Da *da;
  bool found;
} da_present_closure;
  
  bool f[4];
  da_present_closure dpc;
  dpc.da = dap_0_pt;
  dpc.found = false;

  //test da_equal
  f[0] = da_equal(dap_0_pt, dpc.da);
  
  //test da_present
  da_present(dar, dar_size, &dpc);
  f[1] = dpc.found;
  dpc.found = false;
  da_present(dar, dar_size, &dpc);
  f[2] = dpc.found;
  dpc.found = false;
  da_present(dar, dar_size, &dpc);
  f[3] = dpc.found;
  dpc.found = false;

  bool result = f[0] && f[1] && f[2] && f[3];
  
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
-da_strings_exists_0                 
da_push_alloc
-da_push                   
-da_pop                    
da_endq                     
-da_map           Had to put da_test_map function in da src code...
da_free_elements            
da_ints_print               
da_strings_print            
da_strings_exists           
da_strings_set_insert       
da_strings_set_union        
-da_string_input           
da_string_push              
-da_cat                    
da_exists
da_all

*/

/*
  Tests
test_da_push_0
test_da_expand_0
test_da_string_input_0
test_da_pop_0
test_da_cat_0
test_da_cat_1
test_da_rewind_0
test_da_index_0
test_da_free_elements_0
test_da_strings_exists_0
test_da_rebase_0
test_da_boundq_0
test_da_map_0
test_da_present_0

*/
