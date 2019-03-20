/*
Dynamic Array

*/

#include "da.lib.h"

#if INTERFACE
#include<stdlib.h>
#include<stdbool.h>
#endif
#include<string.h>

//--------------------------------------------------------------------------------
// generic
// We manipulate pointers to a smallest addressable unit. The sizeof operator
// returns counts in these addressable units. Sizeof(char) is defined to be 1.

#if INTERFACE
struct da{
  char *base;
  char *end; // one byte/one item off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t item_size;
};
#endif

void da_alloc(da *dap, size_t item_size){
  dap->size = 4 * item_size;
  dap->item_size = item_size;
  dap->end = 0;
  dap->base = malloc(dap->size);
}

// Doubles size of of da.  Returns old base, so that existing pointers into the
// array can be moved to the new array
char *da_expand(da *dap){
  size_t end_offset = ((char *)dap->end - (char *)dap->base);
  size_t new_size = dap->size << 1;
  char *old_base = dap->base;
  char *new_base = malloc( new_size );
  memcpy( new_base, dap->base, end_offset + 1);
  free(old_base);
  dap->base = new_base;
  dap->end = new_base + end_offset;
  dap->size = new_size;
  return old_base;
}

void da_rebase(da *dap, char *old_base, void *pta){
  char **pt = (char **)pta;
  size_t offset = *pt - old_base;
  *pt = dap->base + offset;
}

// true when pt has run off the end of the area currently allocated for the array
bool da_endq(da *dap, void *pt){
  return (char *)pt >= dap->end;
}

// true when pt has run off the end of the area allocated for the array
bool da_boundq(da *dap, void *pt){
  return (char *)pt >= dap->base + dap->size;
}

void da_push(da *dap, void *item){
  if( dap->end + dap->item_size >= dap->base + dap->size ) da_expand(dap);
  memcpy(dap->end, item, dap->item_size);
  dap->end += dap->item_size;
}

// passed in f(item_pt, arg_pt)
// We have no language support closures, so we pass in an argument for it.
// The closure may be set to NULL if it is not needed.
void da_map(da *dap, void f(void *, void *), void *closure){
  char *pt = dap->base;
  while( pt != dap->end ){
    f(pt, closure);
  pt += dap->item_size;
  }
}

// da_lists are sometimes used as resource managers
void da_free(void *pt, void *closure){
  free(pt);
}

#if INTERFACE
#define RETURN(dap, r)                      \
  { da_map(dap, da_free, NULL); return r; }
#endif
  
