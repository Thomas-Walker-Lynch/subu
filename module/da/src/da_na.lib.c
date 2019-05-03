/*
Alternative versions of the da array constructors and destructors for avoiding the accounting
versions of malloc and free.

 */

#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include "da.lib.h"
#include "da_na.lib.h"

#undef MALLOC 
#undef FREE
#define MALLOC malloc
#define FREE free

#define da_alloc da_na_alloc
#define da_free da_na_free
#define da_expand da_na_expand
#define da_push_alloc da_na_push_alloc
#define da_push da_na_push
#define da_pts_free_all_0 da_na_pts_free_all_0
#define da_pts_free_all da_na_pts_free_all

// the following is just cut and paste from da.lib.c 
// ...

void da_alloc(Da *dap, size_t element_size){
  dap->element_size = element_size;
  dap->size = 4 * element_size;
  dap->base = MALLOC(dap->size);
  dap->end = dap->base;
}
void da_free(Da *dap){
  FREE(dap->base);
  dap->size = 0;
}

char *da_expand(Da *dap){
  char *old_base = dap->base;
  size_t end_offset = dap->end - old_base;
  size_t new_size = dap->size << 1;
  char *new_base = MALLOC( new_size );
  memcpy( new_base, old_base, end_offset + dap->element_size);
  FREE(old_base);
  dap->base = new_base;
  dap->end = new_base + end_offset;
  dap->size = new_size;
  return old_base;
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

static void da_pts_free_all_0(void *pt, void *closure){
  FREE(*(char **)pt); // FREE does not care about the pointer type
}
void da_pts_free_all(Da *dap){
  da_map(dap, da_pts_free_all_0, NULL);
  da_rewind(dap);
}

