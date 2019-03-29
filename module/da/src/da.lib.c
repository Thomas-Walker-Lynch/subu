/*
Dynamic Array

*/

#include "da.lib.h"

#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

//--------------------------------------------------------------------------------
// generic
// We manipulate pointers to a smallest addressable unit. The sizeof operator
// returns counts in these addressable units. Sizeof(char) is defined to be 1.

void da_alloc(Da *dap, size_t element_size){
  dap->element_size = element_size;
  dap->size = 4 * element_size;
  dap->base = malloc(dap->size);
  dap->end = dap->base;
}
void da_free(Da *dap){
  free(dap->base);
  dap->size = 0;
}
void da_rewind(Da *dap){
  dap->end = dap->base;
}

bool da_empty(Da *dap){
  return dap->end == dap->base;
}

void da_rebase(Da *dap, char *old_base, void *pta){
  char **pt = (char **)pta;
  size_t offset = *pt - old_base;
  *pt = dap->base + offset;
}

// Doubles size of of da.  Returns old base, so that existing pointers into the
// array can be moved to the new array
char *da_expand(Da *dap){
  char *old_base = dap->base;
  size_t end_offset = dap->end - old_base;
  size_t new_size = dap->size << 1;
  char *new_base = malloc( new_size );
  memcpy( new_base, old_base, end_offset + dap->element_size);
  free(old_base);
  dap->base = new_base;
  dap->end = new_base + end_offset;
  dap->size = new_size;
  return old_base;
}

// true when pt has run off the end of the area currently allocated for the array
bool da_endq(Da *dap, void *pt){
  return (char *)pt >= dap->end;
}

// true when pt has run off the end of the area allocated for the array
bool da_boundq(Da *dap){
  return dap->end >= dap->base + dap->size;
}

void da_push(Da *dap, void *element){
  if( dap->end >= dap->base + dap->size ) da_expand(dap);
  memcpy(dap->end, element, dap->element_size);
  dap->end += dap->element_size;
}

bool da_pop(Da *dap, void *element){
  bool flag = dap->end >= dap->base + dap->element_size;
  if( flag ){
    dap->end -= dap->element_size;
    if(element) memcpy(element, dap->end, dap->element_size);
  }
  return flag;
}

void da_cat(Da *dap0, Da *dap1){
  if(dap1->base == dap1->end) return;
  size_t dap0_size = dap0->end - dap0->base;
  size_t dap1_size = dap1->end - dap1->base; // size of the active portion
  dap0->end += dap1_size;
  while( dap0->end >= dap0->base + dap0->size ) da_expand(dap0);
  memcpy(dap0->base + dap0_size, dap1->base, dap1_size);
}

// If dap0 has had a terminatating zero added, that must be popped off before
// the call.  Similarly if a terminating zero is desired, it should be pushed
// after the call.
void da_push_string(Da *dap0, char *string){
  if(!*string) return;
  size_t dap0_size = dap0->end - dap0->base;
  size_t string_size = strlen(string);
  dap0->end += string_size;
  while( dap0->end >= dap0->base + dap0->size ) da_expand(dap0);
  memcpy(dap0->base + dap0_size, string, string_size);
}


char *da_index(Da *dap, size_t i){
  size_t offset = i * dap->element_size;
  char *pt = dap->base + offset;
  return pt;
}

// passed in f(element_pt, arg_pt)
// We have no language support closures, so we pass in an argument for it.
// The closure may be set to NULL if it is not needed.
void da_map(Da *dap, void f(void *, void *), void *closure){
  char *pt = dap->base;
  while( pt != dap->end ){
    f(pt, closure);
  pt += dap->element_size;
  }
}

// da_lists are sometimes used as resource managers
static void da_free_element(void *pt, void *closure){
  free(*(char **)pt); // free does not care about the pointer type
}

void da_free_elements(Da *dap){
  da_map(dap, da_free_element, NULL);
  da_rewind(dap);
}

// for the case of an array of strings
void da_strings_puts(Da *dap){
  char *pt = dap->base;
  while( pt < dap->end ){
    puts(*(char **)pt);
  pt += dap->element_size;
  }
}

// would like to pass in the printf format to make a general print
// but can't get *pt on the stack for the printf call .. hmmm
void da_ints_print(Da *dap){
  char *pt = dap->base;
  while( pt < dap->end ){
    printf("%u\n", *(int *)pt);
  pt += dap->element_size;
  }
}


// Puts text from a line into buffer *dap. Does not push EOF or '\n' into the
// buffer.  Returns the old_base so that external pointers can be rebased.
// It is possible that the the base hasn't changed. Use feof(FILE *stream) to
// test for EOF;
char *da_fgets(Da *dap, FILE *file){
  char *old_base = dap->base;
  int c = fgetc(file);
  while( c != EOF && c != '\n' ){
    da_push(dap, &c);
  c = fgetc(file);
  }
  int terminator = 0;
  da_push(dap, &terminator);
  return old_base;
}
