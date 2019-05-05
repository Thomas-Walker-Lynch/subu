/*
dynamic memory accounting

Alternative version of malloc and free that will invoke accounting for pointers that are allocated
but not freed, or freed but not allocated.

*/

//new sed file for changes to rest of project
//da_alloc -> da_init
//da_map -> da_foreach

#include "da.lib.h"
#include "da_na.lib.h"
#include "acc.lib.h"

Da *acc_live_checkers = NULL;

void acc_init(){ //open
  if( acc_live_checkers == NULL ){
    acc_live_checkers = malloc(sizeof(Da));
    da_na_init(acc_live_checkers, sizeof(AccChecker));
  }
  AccChecker *acp = malloc(sizeof(AccChecker));
  da_na_alloc(&(acp->outstanding_malloc), sizeof(void *));
  da_na_alloc(&(acp->spurious_free), sizeof(void *));
  da_na_push(acc_live_checkers, acp);
}
void acc_free(){//close
  
}
void acc_report(){
}

void *acc_malloc(size_t mem_size){
  void *an_allocation_pt = malloc(mem_size);

  // push an_allocation_pt on to all live checkers
  AccChecker *acp = acc_live_checkers->base;
  while(!da_endq(acc_live_checkers, acp)){
    if(*acp) da_na_push(&(acp->outstanding_malloc), an_allocation_pt);
  acp++;
  }

  return an_allocation_pt;
}

void *acc_malloc(size_t mem_size){//pushes pointer onto heap_acc before mallocing
  void *temp = malloc(mem_size);
  da_na_push(&heap_acc, &temp);
  return (void *)temp;
}
void acc_free(void *pt){
  if( accounting == true ) {
    void *i = heap_acc.base;
    bool present = false;
    while( i < (void *)heap_acc.end ){
      if( pt == *(void **)i ){//matches and zeroes out pointer from heap_acc
        *(int *)i = 0;
        present = true;
        if( (i + heap_acc.element_size) == heap_acc.end ){//pops excess 0s from end of heap_acc
          void *j = i;
          while( (*(int *)j == 0) && ((void *)j >= (void *)heap_acc.base) ){
            da_pop(&heap_acc, NULL);
          j-=heap_acc.element_size;
          }}}
    i++;
    }
    if( present == false ) da_push(&extra_frees, &pt);//stores pointer in extra_frees if tried to free one that we didn't count
  }
  free(pt);//frees pointer
}
bool acc_result(){//returns false if accounting wasn't being used or heap_acc is not empty or if we tried to free more pointers than we malloced for
  if ( accounting == true ){
    bool flag1 = da_empty(&heap_acc);
    bool flag2 = da_empty(&extra_frees);
    return flag1 && flag2;
  } else return false;
}

