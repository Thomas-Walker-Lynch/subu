/*
Dynamic Array

Cannot expand an empty array.

*/
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include "acc.lib.h"
#include "da.lib.h"

//--------------------------------------------------------------------------------
// constructors / destructors

Da *da_init(Da *dap, size_t element_size, AccChannel *channel){
  dap->element_size = element_size;
  dap->size = 4 * element_size;
  dap->base = acc_malloc(dap->size, channel);
  dap->end = dap->base;
  dap->channel = channel;
  return dap;
}
void da_free(Da *dap){
  acc_free(dap->base, dap->channel);
  dap->size = 0;
  dap->channel = NULL;
}
void da_rewind(Da *dap){
  dap->end = dap->base;
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
  char *new_base = acc_malloc(new_size, (AccChannel *)(dap->channel));
  memcpy( new_base, old_base, end_offset + dap->element_size);
  acc_free(old_base, (AccChannel *)(dap->channel));
  dap->base = new_base;
  dap->end = new_base + end_offset;
  dap->size = new_size;
  return old_base;
}

// true when end has run off the allocated area
bool da_boundq(Da *dap){
  return dap->end > (dap->base + dap->size);
}

// erases all the elements in the array
// a push followed by an erase should work, but I don't think it does right now
/*
void da_erase(Da *dap){
  da_free(Da *dap);
}
*/

//--------------------------------------------------------------------------------
// status / attributes

bool da_is_empty(Da *dap){
  return dap->end == dap->base;
}

bool da_equal(Da *da_0, Da *da_1){
  return !bcmp(da_0, da_1, sizeof(Da));
}

size_t da_length(Da *dap){
  return (dap->end - dap->base)/dap->element_size;
}

bool da_length_equal(Da *dap0, Da *dap1){
  return da_length(dap0) == da_length(dap1);
}


//--------------------------------------------------------------------------------
// accessing

char *da_index(Da *dap, size_t i){
  size_t offset = i * dap->element_size;
  char *pt = dap->base + offset;
  return pt;
}

// allocate space for a new element at the end of the array
static char *da_push_alloc(Da *dap){
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

// array is a row of elements, pt points at the rightmost element
bool da_right_bound(Da *dap, void *pt){
  return ((char *)pt + dap->element_size) >= dap->end;
}

// passed in f(element_pt, arg_pt)
// We have no language support closures, so we pass in an argument for it.
// The closure may be set to NULL if it is not needed.
void da_foreach(Da *dap, void f(void *, void *), void *closure){
  char *pt = dap->base;
  while( pt != dap->end ){
    f(pt, closure);
    pt += dap->element_size;
  }
}

//∃, OR map
//---> should return the element pointer, same for da_pts_exists
static bool da_quantifier(bool complement, Da *dap, bool pred(void *, void*), void *closure){
  char *pt = dap->base;
  bool result = !complement;
  while( (complement? !result : result) && (pt != dap->end) ){
    result = pred(pt, closure);
    pt += dap->element_size;
  }
  return result;
}
bool da_exists(Da *dap, bool pred(void *, void*), void *closure){
  return da_quantifier(true, dap, pred, closure);
}//return pointer to thing it found that exists
bool da_exception(Da *dap, bool pred(void *, void*), void *closure){
  return da_quantifier(false, dap, pred, closure);
}//make return pointer to exception and NULL (pointer) if no exception






//--------------------------------------------------------------------------------
// elements are pointers
// elements are pointers
//
static bool da_pts_exists_0(void *element, void *test_element){ return element == test_element; }
void *da_pts_exists(Da *dap, void *test_element){
  if( da_exists(dap, da_pts_exists_0, test_element) ) return test_element;
  else return NULL; //if da_exists returns pointer, gets bool and pointer in one
}
/*
static da_pts_exists_0(void *element, void *pt){ return element == pt; }
bool da_pts_exists(Da *dap, void *test_element){
  return da_exists(dap, da_pts_exists_0, test_element);
}
*/
// elements were allocated, now they will all be freed
static void da_pts_free_all_0(void *pt, void *closure){
  acc_free(*(char **)pt, closure);
  //  FREE(*(char **)pt); // FREE does not care about the pointer type
}
void da_pts_free_all(Da *dap){
  da_foreach(dap, da_pts_free_all_0, dap->channel);
  //  da_map(dap, da_pts_free_all_0, NULL);
  da_rewind(dap);
}

// elements are pointers
// ept points at an element, we set *ept to NULL
// we pop all NULLs off the top of the stack
void da_pts_nullify(Da *dap, void **ept){
  if(ept >= (void **)(dap->base) && ept < (void **)(dap->end)){
  //  if(ept >= dap->base && ept < dap->end){
    *ept = NULL;
  }
  while(
        dap->end > dap->base
        &&
        //        *(void **)(dap->end - dap->element_size) == NULL
        //        ){
        *(void **)(dap->end - dap->element_size) == NULL){
    da_pop(dap, NULL);
  }
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

bool da_integer_repeats(Da *dap){//all items in the array are equal
  int n = *(dap->base);
  char *pt = dap->base + dap->element_size;
  bool flag = true;
  while( flag && pt != dap->end ){
    flag = *pt == n;
  pt+=dap->element_size;
  }
  return flag;
}

int da_integer_sum(Da *dap){//sum all elements
  char *pt = dap->base;
  int sum = 0;
  while( pt != dap->end ){
    sum += *(pt);
  pt+=dap->element_size;
  }
  return sum;
}




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
  char *string_element = *(char **)sp;
  da_strings_exists_closure *ss = (da_strings_exists_closure *)closure;
  if( ss->found ) return;
  ss->found = !strcmp(string_element, ss->string);
  return;
}

bool da_strings_exists(Da *string_arrp, char *test_string){
  da_strings_exists_closure sec;
  sec.string = test_string;
  sec.found = false;
  da_foreach(string_arrp, string_equal, &sec);
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


