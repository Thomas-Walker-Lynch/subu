#include "da.lib.h"

#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

//------------------------------------------------------------------
//FREE and MALLOC replacement

static bool accounting = false;
void da_start_accounting(Da heap_acc, Da extra_frees, bool acc){//assigns properties of heap_acc and extra_frees, must be called before other code to be used
  heap_acc.element_size = sizeof(void *);
  heap_acc.size = 4*sizeof(void *);
  heap_acc.base = malloc(heap_acc.size);
  heap_acc.end = heap_acc.base;
  extra_frees.element_size = sizeof(void *);
  extra_frees.size = 4*sizeof(void *);
  extra_frees.base = malloc(extra_frees.size);
  extra_frees.end = extra_frees.base;
  acc = true;
}
void *da_malloc_counted(size_t mem_size){//pushed pointer onto heap_acc
  void *temp = malloc(mem_size);
  if( accounting == true ) da_na_push(&heap_acc, &temp);
  return (void *)temp;
}
void da_free_counted(void *pt){
    if( accounting == true ) {
      void *i = heap_acc.base;
      bool present = false;
      while( i < (void *)heap_acc.end ){
        if( pt == *(void **)i ){//matches and zeroes out pointer from heap_acc
          *(int *)i = 0;
          present = true;
          if( (i + heap_acc.element_size) == heap_acc.end )//pops excess 0s from end
            da_pop(&heap_acc, NULL);
        }
      i++;
      }
      if( present == false ) da_push(&extra_frees, &pt);//stores pointer in extra_frees if tried to free one that we didn't count
    }
    free(pt);
}
//returns false if accounting wasn't being used or heap_acc is not empty or if we tried to free more pointers than we malloced for
bool da_result_accounting(){
  if ( accounting == true ){
    bool flag1 = da_empty(&heap_acc);
    bool flag2 = da_empty(&extra_frees);
    return flag1 && flag2;
  } else return false;
}

//these are redefined with malloc instead of MALLOC because da_malloc_counted will not make it past the push once heap_acc needs to expand
char *da_na_expand(Da *dap){
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
char *da_na_push_alloc(Da *dap){
  size_t element_off = dap->end - dap->base;
  dap->end += dap->element_size;
  if( dap->end > dap->base + dap->size ) da_na_expand(dap);
  return dap->base + element_off;
}
char *da_na_push(Da *dap, void *element){
  char *element_pt = da_na_push_alloc(dap);
  memcpy(element_pt, element, dap->element_size);
  return element_pt;
}
void da_na_free(Da *dap){
  free(dap->base);
  dap->size = 0;
}
