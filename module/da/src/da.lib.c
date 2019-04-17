/*
Dynamic Array

*/
#include "da.lib.h"

#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

//--------------------------------------------------------------------------------
// allocation

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

bool da_emptyq(Da *dap){
  return dap->end == dap->base;
}

size_t da_length(Da *dap){
  return (dap->end - dap->base)/dap->element_size;
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

// true when end has run off the allocated area
bool da_boundq(Da *dap){
  return dap->end > (dap->base + dap->size);
}

//--------------------------------------------------------------------------------
// putting/taking items

char *da_index(Da *dap, size_t i){
  size_t offset = i * dap->element_size;
  char *pt = dap->base + offset;
  return pt;
}

// allocate space for a new element at the end of the array
char *da_push_alloc(Da *dap){
  size_t element_off = dap->end - dap->base;
  dap->end += dap->element_size;
  if( dap->end > dap->base + dap->size ) da_expand(dap);
  return dap->base + element_off;
}

char *da_push(Da *dap, void *element){
  char *element_pt = da_push_alloc(dap);
  memcpy(element_pt, element, dap->element_size);
  return element_pt;
}

bool da_pop(Da *dap, void *element){
  bool flag = dap->end >= dap->base + dap->element_size;
  if( flag ){
    dap->end -= dap->element_size;
    if(element) memcpy(element, dap->end, dap->element_size);
  }
  return flag;
}

//--------------------------------------------------------------------------------
// iteration

// true when pt has run off the end
bool da_endq(Da *dap, void *pt){
  return (char *)pt >= dap->end;
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

void da_test_map(void *pt, void *closure){
  bool *f1 = (bool *)closure;
  *f1 = true;
}


//--------------------------------------------------------------------------------
// da being used as a resource manager


// elements were malloced, now they will all be freed
static void da_free_element(void *pt, void *closure){
  free(*(char **)pt); // free does not care about the pointer type
}

void da_free_elements(Da *dap){
  da_map(dap, da_free_element, NULL);
  da_rewind(dap);
}

//--------------------------------------------------------------------------------
// da is an array of integers

// would like to pass in the printf format to make a general print
// but can't get *pt on the stack for the printf call .. hmmm
void da_ints_print(Da *dap, char *sep){
  char *pt = dap->base; // char * because it points to a byte in the array
  if( pt < dap->end ){
    printf("%u", *(int *)pt);
    pt += dap->element_size;
    while( pt < dap->end ){
      printf("%s%u", sep, *(int *)pt);
      pt += dap->element_size;
    }}}


//--------------------------------------------------------------------------------
// da is an array of strings

// for the case of an array of strings
void da_strings_print(Da *dap, char *sep){
  char *pt = dap->base; // char * because it points to a byte in the array
  if( pt < dap->end ){
    fputs(*(char **)pt, stdout);
    pt += dap->element_size;
    while( pt < dap->end ){
      fputs(sep,stdout);
      fputs(*(char **)pt,stdout);
      pt += dap->element_size;
    }}}

// da is an array of strings, true if the test string is in the array
// might be better to iterate instead of using a map ... 
typedef struct {
  char *string;
  bool found;
} da_strings_exists_closure;
static void string_equal(void *sp, void *closure){
  const char *string_element = *(char **)sp;
  da_strings_exists_closure *ss = (da_strings_exists_closure *)closure;
  const char *str = ss->string;
  if( ss->found ) return;
  ss->found = !strcmp(string_element, str);
  return;
}
//added consts to see if it was a strcmp syntax issue

/*original
static void string_equal(void *sp, void *closure){
  char *string_element = *(char **)sp;
  da_strings_exists_closure *ss = (da_strings_exists_closure *)closure;
  if( ss->found ) return;
  ss->found = !strcmp(string_element, ss->string);
  return;
}
*/

bool da_strings_exists(Da *string_arrp, char *test_string){
  da_strings_exists_closure sec;
  sec.string = test_string;
  sec.found = false;
  da_map(string_arrp, string_equal, &sec);
  return sec.found;
}

void da_strings_set_insert(Da *string_arrp, char *proffered_string, void destruct(void *)){
  if( da_strings_exists( string_arrp, proffered_string)){ // then throw it away, we don't need it
    if(destruct)destruct(proffered_string);
    return;
  }
  da_push(string_arrp, &proffered_string);
}

// union
void da_strings_set_union(Da *string_arrp, Da *proffered_string_arrp, void destruct(void *)){
  char *pt = proffered_string_arrp->base;
  while( pt < proffered_string_arrp->end ){
    da_strings_set_insert(string_arrp, *(char **)pt, destruct);
  pt += proffered_string_arrp->element_size;
  }
  return;
}


//--------------------------------------------------------------------------------
// the da itself is a string

// Puts text from a line into buffer *dap. Does not push EOF or '\n' into the
// buffer.  Returns the old_base so that external pointers can be rebased.
// It is possible that the the base hasn't changed. Use feof(FILE *stream) to
// test for EOF;
char *da_string_input(Da *dap, FILE *file){
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

void da_string_push(Da *dap0, char *string){
  if(!*string) return;
  size_t dap0_size = dap0->end - dap0->base;
  size_t string_size = strlen(string);
  dap0->end += string_size;
  while( dap0->end >= dap0->base + dap0->size ) da_expand(dap0);
  memcpy(dap0->base + dap0_size, string, string_size);
}


//--------------------------------------------------------------------------------
// list operations

// If dap0 has had a terminatating zero added, that must be popped off before
// the call.  Similarly if a terminating zero is desired, it should be pushed
// after the call.

// appends contents of dap1 onto dap0
void da_cat(Da *dap0, Da *dap1){
  if(dap1->base == dap1->end) return;
  size_t dap0_size = dap0->end - dap0->base;
  size_t dap1_size = dap1->end - dap1->base; // size of the active portion
  dap0->end += dap1_size;
  while( dap0->end >= dap0->base + dap0->size ) da_expand(dap0);
  memcpy(dap0->base + dap0_size, dap1->base, dap1_size);
}

//array of Das

//∃, OR map
//checks that some Da is present in Da of Das

typedef struct{
  Da *da;
  bool found;
} da_exists_closure;

void da_present(void *da_el, void *closure){
  Da *da_element = (Da *)da_el;
  da_exists_closure *dd = (da_exists_closure *)closure;
  Da *test_element = dd->da;
  if (dd->found) return;
  dd->found = da_equal(da_element, test_element);
  return;
}//may need to be static?

bool da_equal(Da *da_el, Da *test_el){
  bool f1 = da_el->base == test_el->base;
  bool f2 = da_el->end == test_el->end;
  bool f3 = da_el->size == test_el->size;
  bool f4 = da_el->element_size == test_el->element_size;
  return f1 && f2 && f3 && f4;
}
  
bool da_exists (Da **dar, int dar_size, Da *dap){
  da_exists_closure dec;
  Da *da_first = *dar;
  dec.da = dap;
  dec.found = false;
  da_big_map(dar, dar_size, da_present, &dec);
  return dec.found;
}

void da_big_map(Da **dar, int dar_size, void f(void *, void *), void *closure){
  Da *pt = *dar;
  int i = 0;
  while( i < dar_size ){
    f(pt, closure);
    pt++;
    i++;
  }
  return;
}

//∀, AND map
//checks that all Das are present in Da of Das
bool da_all (Da **dar, int dar_size, Da **dap){
  da_exists_closure dec;
  dec.da = *dap;
  dec.found = true;
  int i = 0;
  while(dec.found && (i < dar_size)){
    da_big_map(dar, dar_size, da_present, &dec);
    dec.da++;
    i++;
  }
  return dec.found;
}
