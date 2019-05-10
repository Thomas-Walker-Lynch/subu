/*
Tests for Da.

*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <da.h>
#include <acc.h>

#include "test_da.lib.h"


// tests push
bool test_da_push_0(){
  Da da;
  da_init(&da, sizeof(int), NULL); // leaves room for 4 ints
  int i = 0;
  int *pt = (int *)da.base;
  // will double, 4 -> 8, then double 8 -> 16
  while( i < 10 ){
    da_push(&da, &i);
  i++;
  pt++;
  }

  bool flag0 = da.size == sizeof(int) * 16;
  bool flag1 = 10 == (da.end - da.base) / sizeof(int);
  bool flag2 = true;
  pt = (int *)da.base;
  i = 0;
  while( i < 10 ){
    flag2 = flag2 && *pt == i && !da_endq(&da, pt);
  i++;
  pt++;
  }
  bool flag3 = da_endq(&da, pt);
  da_free(&da);
  return flag0 && flag1 && flag2 && flag3;
}

// tests manual expansion
bool test_da_expand_0(){
  Da da;
  da_init(&da, sizeof(int), NULL); // leaves room for 4 ints
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

  bool flag0 = da.size == sizeof(int) * 16;
  bool flag1 = 10 == (da.end - da.base) / sizeof(int);
  bool flag2 = true;
  pt = (int *)da.base;
  i = 0;
  while( i < 10 ){
    flag2 = flag2 && *pt == i && !da_endq(&da, pt);
  i++;
  pt++;
  }
  bool flag3 = da_endq(&da, pt);
  da_free(&da);
  return flag0 && flag1 && flag2 && flag3;
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
  da_init(&da, sizeof(char), NULL);

  da_string_input(&da, file);
  bool flag0 = !strcmp(da.base, "this is a test");

  char *old_base;
  da_pop(&da, NULL); // pop the prior null terminator
  char *s1 = da.end;
  old_base = da_string_input(&da,file);
  da_rebase(&da, old_base, &s1);
  bool flag1 = !strcmp(s1, "ends without a newline");
  
  da_pop(&da, NULL); // pop the prior null terminator
  char *s2 = da.end;
  old_base = da_string_input(&da,file);
  da_rebase(&da, old_base, &s2);
  bool flag2 = !strcmp(s2, "(setq mode-require-final-newline nil)");

  bool flag3 = !strcmp(da.base, "this is a testends without a newline(setq mode-require-final-newline nil)");

  fclose(file);
  da_free(&da);
  return flag0 && flag1 && flag2 && flag3;
}

// da_pop
bool test_da_pop_0(){

  Da da;
  da_init(&da, sizeof(int), NULL);

  int i = 5;
  da_push(&da, &i);
  i++;
  da_push(&da, &i);
  i++;
  da_push(&da, &i);

  int j;
  bool flag0 = da_pop(&da, &j) && j == 7;
  bool flag1 = da_pop(&da, &j) && j == 6;
  bool flag2 = da_pop(&da, NULL);
  bool flag3 = !da_pop(&da, &j);
  da_free(&da);
  return flag0 && flag1 && flag2 && flag3;
}

// da_cat
bool test_da_cat_0(){

  Da da0, da1;
  da_init(&da0, sizeof(int), NULL);
  da_init(&da1, sizeof(int), NULL);

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

  bool flag[6];
  int j;
  int k = 0;
  while(k < 6){
    flag[k] = da_pop(&da0, &j) && (j == 10 - k);
  k++;
  }

  bool result = flag[0];
  k = 1;
  while(result && k < 6){
    result = flag[k];
  k++;
  }
  da_free(&da0);
  da_free(&da1);
  return result;
}

//Glenda: this was already tested much better above, where each element was actually tested, not just one, but it was a good one to test for practice
bool test_da_cat_1(){
  Da dar0;
  Da dar1;
  da_init(&dar0, sizeof(int), NULL);
  da_init(&dar1, sizeof(int), NULL);
  int n = 2;
  {
    int m = 0;
    while(m < 4){
      da_push(&dar0, &n);
    n+=2;
    m++;
    }
  }
  size_t off0 = dar0.end - dar0.base;
  bool flag1 = dar0.base != dar0.end;
  for (int m=0; m<4; m++){
    da_push(&dar1, &n);
    n+=2;
  }
  size_t off1 = dar1.end - dar1.base;
  bool flag2 = dar1.base != dar1.end;
  da_cat(&dar0, &dar1);
  bool flag3 = dar0.end == dar0.base + off0 +off1;
  bool flag4 = *(dar0.base + (5*dar0.element_size)) == *(dar1.base + dar1.element_size);
  da_free(&dar0);
  da_free(&dar1); 
  return flag1 && flag2 && flag3 && flag4;
}


//Glenda's tests
//tested output for success and artificial failure

//tests da_rewind
bool test_da_rewind_0(){
  int i = 10;
  Da dar;
  da_init(&dar, sizeof(int), NULL);
  while(i < 18){
    da_push(&dar, &i);
  ++i;
  }
  bool flag1 = dar.end != dar.base;
  da_rewind(&dar);
  int n = *(dar.end);
  bool flag2 = dar.end == dar.base;
  bool flag3 = n == 10;
  bool flag4 = (*(dar.end + (7 * sizeof(int))) = 17);
  bool result = flag1 && flag2 && flag3 && flag4;

  da_free(&dar);
  return result;
}

//tests da_index
bool test_da_index_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_init(dar_pt, sizeof(int), NULL);

  
  bool flag[4];
  {//push to dar and test da_index
    char *a[4];
    size_t i = 0;
    int n = 10;
    int *n_pt = &n;
    while(i < 4){
      da_push(dar_pt, n_pt);
      a[i] = da_index(dar_pt, i);
      flag[i] = a[i] == dar.base + (i*dar.element_size);
    i++;
    n++;
    }
  }
  bool result = flag[0] && flag[1] && flag[2] && flag[3];
  da_free(dar_pt); 
  return result;
}

//tests da_free_elements - still getting core dump on function call
//cannot test sub-functions bc da_free_element needs to be static
bool test_da_free_elements_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_init(dar_pt, sizeof(int*), NULL);

  int i = 3;
  int *i_pt = &i;
  while(i < 7){
    da_push(dar_pt, &i_pt);
  ++i;
  }
  
  //da_free_elements(dar_pt);
  bool result = true;
  da_free(dar_pt); 
  return result;
}

//tests da_strings_exist
bool test_da_strings_exists_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_init(dar_pt, sizeof(char *), NULL);

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
  da_free(dar_pt); 
  return result;
}


//tests rebase 
bool test_da_rebase_0(){
  Da dar;
  da_init(&dar,sizeof(char), NULL);

  char **el_pt = (char **)acc_malloc(4*(sizeof(char*)), NULL);
  {//push "temp" into dar, leave with pointer to last element
    int i = 0;
    char arr[4] = {'t','e','m','p'};
    while(i<4){
      char c = *(arr+i);
      *el_pt = da_push(&dar, &c);
    el_pt++;
    i++;
    }
    el_pt--;
  }

  //check that push worked, that element pointer is in right place, and that expand works
  bool flag1 = dar.base != dar.end;
  bool flag2 = *el_pt == dar.end - dar.element_size;

  char *obase = dar.base;
  char *old_base = da_expand(&dar);
  bool flag3 = obase == old_base;

  bool flag4[4];
  {//check that each pointer is properly rebased
    int i = 0;
    while(i<4 && *el_pt >= old_base){
      size_t old_offset = *el_pt - old_base;
      da_rebase(&dar, old_base, el_pt);
      size_t new_offset = *el_pt - dar.base;
      flag4[i] = old_offset == new_offset;
    el_pt--;
    i++;
    }
    el_pt++;
  }
  
  bool result = flag4[0];
  {//now check that all pointers are properly rebased
    int i = 1;
    while(result && i < 4){
      result = flag4[i];
    i++;
    }
  }
  da_free(&dar);
  acc_free(el_pt, NULL);
  return flag1 && flag2 && flag3 && result;
}

/* {//problems with FREE and MALLOC
  char *a = MALLOC(10);
  strcpy(a, "zsdf");
  Da da;
  Da *da_pt = &da;
  da_init(da_pt, sizeof(char *));
  da_push(da_pt, a);
  ...
    FREE(*(char *)da_index(da_pt, 0));
    da_free(da_pt);
} */

//tests da_boundq
bool test_da_boundq_0(){
  Da dar;
  da_init(&dar,sizeof(char), NULL);

  bool flag[5];
  {//pushes onto dar and tests no runoff
    char arr[4] = {'r','e','s','t'};
    int i = 0;
    char c;
    while(i<4){
      c = arr[i];
      da_push(&dar, &c);
      flag[i] = (dar.end != dar.base) && !da_boundq(&dar);
    i++;
    }
  
  }

  //makes end pointer run off allocated area, tests boundq detects runoff
  dar.end++; // = dar.base + dar.size;
  flag[4] = da_boundq(&dar);

  bool result = true;
  {//check all test results
    int i = 0;
    while (result && i < 5){
      result = flag[i];
    i++;
    }
  }
  da_free(&dar);
  return result;
}

//tests foreach
static void test_da_foreach_0_helper(void *pt, void *closure){
  int *n = (int *)closure;
  *n += *(int *)pt;
}
bool test_da_foreach_0(){
  Da dar;
  da_init(&dar, sizeof(int), NULL);

  {//pushes onto dar
    int arr[4] = {5,6,7,8};
    int i = 0;
    while(i<4){
      da_push(&dar, arr+i);
    i++;
    }
  }

  int n = 0;
  int *closure = &n;
  
  da_foreach(&dar, test_da_foreach_0_helper, (int *)closure);
  //rename to da_foreach
  bool result = n == (5+6+7+8);
  da_free(&dar);
  return result;
}

/*
//tests da_present
bool test_da_present_0(){
  int dar_size = 0;
  Da **dar = acc_malloc(3 * sizeof(Da *), NULL);

  Da dap_0;
  Da *dap_0_pt = &dap_0;
  da_init(dap_0_pt,sizeof(int), NULL);

  Da dap_1;
  Da *dap_1_pt = &dap_1;
  da_init(dap_1_pt,sizeof(char), NULL);
  
  dar[dar_size] = dap_0_pt; dar_size++;
  dar[dar_size] = dap_1_pt; dar_size++;
  Da dap_2;
  Da *dap_2_pt = &dap_2;
  da_init(dap_2_pt,sizeof(char *), NULL);
  dar[dar_size] = dap_2_pt; dar_size++;  
  
  Da ;
  Da *matrix = ;
  da_init(dar, sizeof(Dap *));
  
  typedef struct{
  Da *da;
  bool found;
} da_present_closure;
  
  bool flag[4];
  da_present_closure dpc;
  dpc.da = dap_0_pt;
  dpc.found = false;

  //test da_equal
  flag[0] = da_equal(dap_0_pt, dpc.da);
  
  //test da_present
  da_present(dar, dar_size, &dpc);
  flag[1] = dpc.found;
  dpc.found = false;
  da_present(dar, dar_size, &dpc);
  flag[2] = dpc.found;
  dpc.found = false;
  da_present(dar, dar_size, &dpc);
  flag[3] = dpc.found;
  dpc.found = false;

  bool result = flag[0] && flag[1] && flag[2] && flag[3];
  da_free(dap_0_pt); da_free(dap_1_pt); da_free(dap_2_pt);
  acc_free(dar, NULL);
  return result;
}
*/

//tests for da_exists and all

static bool test_exists_helper(void *pt, void *closure){
  bool result = *(int*)pt == *(int *)closure;
  return result;
}

//tests da_exists
bool test_da_exists_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_init(dar_pt, sizeof(int), NULL);

  int n[5] = {5,6,7,8,9};
  
  {//pushes ints 5-8 onto dar
    int j = 0;
    while (j < 5){
      if (j != 3){
        int *n_pt = n+j;
        da_push(dar_pt, n_pt);
      }
    j++;
    } 
  }
  
  bool flag[5];
  {//tests da_exists
    int j = 0;
    while(j < 5){
      int *n_pt = n+j;
      flag[j] = da_exists(dar_pt, test_exists_helper, n_pt);
    j++;
    }
  }

  bool result_1 = flag[0] && flag[1] && flag[2] && !flag[3] && flag[4];
  
  {//add 8
    int *n_pt = n+3;
    da_push(dar_pt, n_pt);
  }  

  {//tests da_exists
    int j = 0;
    while(j < 5){
      int *n_pt = n+j;
      flag[j] = da_exists(dar_pt, test_exists_helper, n_pt);
    j++;
    }
  }
  
  bool result_2 = flag[0] && flag[1] && flag[2] && flag[3] && flag[4];
  da_free(dar_pt);  
  return result_1 && result_2;
}
bool test_da_exists_1(){//tests that expansion doesn't change results
  Da dar;
  Da *dar_pt = &dar;
  da_init(dar_pt, sizeof(int), NULL);
  
  int n[8] = {20,21,22,23,24,25,26,27};
  
  {//pushes 20-27 onto dar except 23 and 26 
    int j = 0;
    while (j < 8){
      if (j != 3 && j != 6){
        int *n_pt = n+j;
        da_push(dar_pt, n_pt);
      }
    j++;
    } 
  }
  
 bool flag[8];
  {//tests da_exists
    int j = 0;
    while(j < 8){
      int *n_pt = n+j;
      flag[j] = da_exists(dar_pt, test_exists_helper, n_pt);
    j++;
    }
  }

  bool result_1 =
    flag[0] && flag[1] && flag[2] && !flag[3] && flag[4] && flag[5]
    && !flag[6] && flag[7];
  
  {//add 23 and 26
    int *n_pt = n+3;
    da_push(dar_pt, n_pt);
    n_pt = n+6;
    da_push(dar_pt, n_pt);
  }  

  {//tests da_exists
    int j = 0;
    while(j < 8){
      int *n_pt = n+j;
      flag[j] = da_exists(dar_pt, test_exists_helper, n_pt);
    j++;
    }
  }

  bool result_2 =
      flag[0] && flag[1] && flag[2] && flag[3] && flag[4] && flag[5]
    && flag[6] && flag[7];
  da_free(dar_pt);
  return result_1 && result_2;
}



//tests da_all
bool test_da_all_0(){
  Da dar;
  Da *dar_pt = &dar;
  da_init(dar_pt, sizeof(int), NULL);

  int n = 5;
  int *n_pt = &n;
  //push 5 onto dar 4 times
  da_push(dar_pt, n_pt);
  da_push(dar_pt, n_pt);
  da_push(dar_pt, n_pt);
  da_push(dar_pt, n_pt);
  
  //tests da_all is true
  bool flag1 = da_all(dar_pt, test_exists_helper, n_pt);

  da_pop(dar_pt, NULL);
  n = 6;
  //tests da_all is false
  bool flag2 = !da_all(dar_pt, test_exists_helper, n_pt);
  da_free(dar_pt);
  
  return flag1 && flag2;
}



//tests da_init
bool test_da_init_0(){
  Da da0;    Da *da0_pt = &da0;    da_init(da0_pt, sizeof(char), NULL);
  Da da1;    Da *da1_pt = &da1;    da_init(da1_pt, sizeof(int), NULL);
  Da da2;    Da *da2_pt = &da2;    da_init(da2_pt, sizeof(char *), NULL);
  bool flag[6];

  //check that da is allocated as array of 4 chars
  flag[0] = da0_pt->element_size == 1;
  flag[1] = da0_pt->size == 4;
  
  //check that da is allocated as array of 4 ints
  flag[2] = da1_pt->element_size == 4;
  flag[3] = da1_pt->size == 16;

  //check that da is allocated as array of 4 char *s
  flag[4] = da2_pt->element_size == 8;
  flag[5] = da2_pt->size == 32;

  bool result =
    flag[0] && flag[1] && flag[2] && flag[3] && flag[4] && flag[5];

  da_free(da0_pt);
  da_free(da1_pt);
  da_free(da2_pt);
  return result;
}

//tests da_free
bool test_da_free_0(){
  Da da;
  Da *da_pt = &da;
  da_init(da_pt, sizeof(int), NULL);

  //store location of da
  Da *keep = da_pt;
  Da *save = da_pt;
  
  //acc_free da
  da_free(da_pt);

  //re-allocate memory to dew da of chars
  da_init(keep, sizeof(char), NULL);

    //test that same memory is properly re-allocated 
  bool flag1 = keep == save;
  bool flag2 = keep->element_size == 1;
  da_free(keep);
  return flag1 && flag2;
}

//tests da_is_empty
bool test_da_is_empty_0(){
  int i = 6;
  Da da;
  Da *da_pt = &da;
  da_init(da_pt, sizeof(int), NULL);
  while(i < 11){
    da_push(da_pt, &i);
  ++i;
  }
  bool flag1 = !da_is_empty(da_pt);
  da_rewind(da_pt);
  bool flag2 = da_is_empty(da_pt);
  da_free(da_pt);
  return flag1 && flag2;
}

//tests da_length
bool test_da_length_0(){
 int i = 1;
  Da da;
  Da *da_pt = &da;
  da_init(da_pt, sizeof(int), NULL);

  //test da_length, even pushing past expansions
  bool result = true;
  while(result && i < 15){
    da_push(da_pt, &i);
    size_t length = da_length(da_pt);
    result = length == i;
  ++i;
  }
  da_free(da_pt);
  return result;
}

/*
//------------------------------------------------
// Matrix function tests

//tests da_push_row
bool test_da_push_row_0(){
  Da dama;   da_init(&dama, sizeof(Da), NULL);   Da *damp = &dama;
  Da row0;   da_init(&row0, sizeof(int), NULL);  da_push_row(damp, &row0);
  Da row1;   da_init(&row1, sizeof(int), NULL);  da_push_row(damp, &row1);
  Da row2;   da_init(&row2, sizeof(int), NULL);  da_push_row(damp, &row2);

  bool flag0 = da_equal(&row0, (Da *)(damp->base));
  bool flag1 = da_equal(&row1, (Da *)(damp->base + damp->element_size));
  bool flag2 = da_equal(&row2, (Da *)(damp->base + 2*(damp->element_size)));
  int n = 5;
  while( n < 8 ){
    da_push(&row0, &n);
  n++;
  }
  //  bool flag3 = da_equal(&row0, (Da *)(damp->base));
  //  Dama won't track changes to Das after pushing onto rows
  da_free_elements(damp);
  da_erase(damp);
  return flag0 && flag1 && flag2;// && flag3;
}

//tests da_erase
bool test_da_erase_0(){
  Da dama;   da_init(&dama, sizeof(Da), NULL);   Da *damp = &dama;
  Da row0;   da_init(&row0, sizeof(int), NULL);  da_push_row(damp, &row0);
  Da row1;   da_init(&row1, sizeof(int), NULL);  da_push_row(damp, &row1);
  Da row2;   da_init(&row2, sizeof(int), NULL);  da_push_row(damp, &row2);

  //store location of da
  Da *keep = damp;
  Da *save = damp;
  
  //free da
  da_free_elements(damp);
  da_erase(damp);

  //re-allocate memory to dew da of chars
  da_init(keep, sizeof(char), NULL);

    //test that same memory is properly re-allocated 
  bool flag1 = keep == save;
  bool flag2 = keep->element_size == 1;
  da_free_elements(keep);
  da_free(keep);
  return flag1 && flag2;
}

//tests da_longer
bool test_da_longer_0(){
  Da dama;   Da *damp = &dama;  da_init(damp, sizeof(Da), NULL);
  
  Da row0;   Da *r0 = &row0;    da_init(r0, sizeof(int), NULL);  
  {//fills first row with 4 integers
    int i = 10;
    while(i<14){
      da_push(r0, &i);
    i++;
    }
  }
  da_push_row(damp, r0);
  
  Da row1;   Da *r1 = &row1;    da_init(r1, sizeof(int), NULL);  
  {//fills second row with 4 different integers
    int i = 20;
    while(i<24){
      da_push(r1, &i);
    i++;
    }
  }
  da_push_row(damp, r1);

  Da row2;   Da *r2 = &row2;    da_init(r2, sizeof(int), NULL); 
  {//fills third row with 6 integers
    int i = 30;
    while(i<36){
      da_push(r2, &i);
    i++;
    }
  }
  da_push_row(damp, r2);
  
  //plain test for which is Da is longer
  Da *test1 = da_longer(r0, r1);
  Da *test2 = da_longer(r0, r2);
  
  //tests from dama which row is longer
  Da *dr0 = (Da *)(damp->base);
  Da *dr1 = (Da *)((damp->base)+sizeof(Da *));
  Da *dr2 = (Da *)((damp->base)+(2*sizeof(Da *)));
  Da *test3 = da_longer(dr0, dr1);
  Da *test4 = da_longer(dr2, dr0);
  
  bool flag1 = test1 == r1;
  bool flag2 = test2 == r2;
  bool flag3 = test3 == dr1;
  bool flag4 = test4 == dr2;

  da_free_elements(damp);
  da_erase(damp);
  return flag1 && flag2 && flag3 && flag4; 
}

//tests da_longest
bool test_da_longest_0(){

  Da dama;   Da *damp = &dama;  da_init(damp, sizeof(Da), NULL);
  
  Da row0;   Da *r0 = &row0;    da_init(r0, sizeof(int), NULL);  
  {//fills first row with 4 integers
    int i = 10;
    while(i<14){
      da_push(r0, &i);
    i++;
    }
  }
  da_push_row(damp, r0);
  
  Da row1;   Da *r1 = &row1;    da_init(r1, sizeof(int), NULL);  
  {//fills second row with 4 different integers
    int i = 20;
    while(i<24){
      da_push(r1, &i);
    i++;
    }
  }
  da_push_row(damp, r1);

  Da row2;   Da *r2 = &row2;    da_init(r2, sizeof(int), NULL); 
  {//fills third row with 6 integers
    int i = 30;
    while(i<36){
      da_push(r2, &i);
    i++;
    }
  }
  da_push_row(damp, r2);
  
  Da *dr0 = (Da *)(damp->base);
  Da *dr1 = (Da *)((damp->base)+sizeof(Da));
  Da *dr2 = (Da *)((damp->base)+(2*sizeof(Da)));
  Da *test = da_longest(damp);
  
  bool flag = test == dr2;
  da_free_elements(damp);
  da_erase(damp);
  return flag;
}
*/

bool test_da_accounting_0(){
  AccChannel acc0;
  AccChannel *acc0_pt = acc_open(&acc0, acc_NULL);

  //Da da0;
  //Da *dap = &da0;
  //da_init(dap, sizeof(int), acc0_pt);

  printf("Outstanding Malloc: %d\n", (int)(acc0_pt->outstanding_malloc));
  printf("Spurious Free: %d\n", (int)(acc0_pt->spurious_free)); 
  acc_report(acc0_pt);

  //da_free(dap);
  //acc_close(acc0_pt);

  //acc_close(acc0_pt);
  bool result = true;
  return result;
}



/*need to update list
  Functions               
-da_init                    
-da_free                     
-da_rewind                 
-da_is_empty                   
-da_length                   
-da_rebase                   
-da_expand                 
-da_boundq                   
-da_index 
-da_strings_exists_0                 
-da_push                   
-da_pop                    
da_endq                     
-da_foreach           
da_free_elements            
da_ints_print
da_integer_repeats               
da_strings_print            
-da_strings_exists           
da_strings_set_insert       
da_strings_set_union        
-da_string_input           
da_string_push              
-da_cat                    
-da_exists
-da_all

//matrix
-da_erase
da_push_row_alloc
-da_push_row
da_push_column

da_every_row
-da_longer
-da_longest
da_every_column

da_matrix_transpose

da_length_equal
da_all_rows_same_length
da_integer_all_rows_repeat
da_integer_all_columns_repeat
da_integer_repeats_matrix

da_integer_transpose
da_integer_to_raw_image_matrix
da_integer_to_raw_image_transpose

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
test_da_foreach_0
test_da_present_0
test_da_exists_0
test_da_exists_1
test_da_all_0
test_da_init_0
test_da_free_0
test_da_is_empty_0
test_da_length_0

//matrix
da_push_row_0
da_erase_0
test_da_longer_0
test_da_longest_0

*/
