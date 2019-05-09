/*
  Dynamic Array

  Cannot expand an empty array.

  Accounting built into code. Alternative version of malloc and free that will invoke accounting for pointers that are allocated
  but not freed, or freed but not allocated.

*/

#include "da.lib.h"

//-----------------------------------------------------------------
//function definitions for accounting

Acc_channel *acc_open(Acc_channel *channel, enum Mode mode){//acc init
  if( channel == &acc_live_channels ) {//avoid pushing channel tracker onto itself
    Da os; Da sf;
    channel->outstanding_malloc = da_init(&os, sizeof(void *), NULL);
    channel->spurious_free = da_init(&sf, sizeof(void *), NULL);
    channel->mode = mode;
    return channel;
  }
  else if( acc_live_channels.mode == acc_NULL ){//accounting NULL
    //channel = (Acc_channel *)acc_malloc(sizeof(Acc_channel), NULL);//accounting channel still on the heap but not tracked in SELF mode
    Da os; Da sf;
    channel->outstanding_malloc = da_init(&os, sizeof(void *), NULL);
    channel->spurious_free = da_init(&sf, sizeof(void *), NULL);
    channel->mode = mode;
    return channel;
  }
  else if( acc_live_channels.mode == acc_SELF ){//accounting tracks itself
    channel = (Acc_channel *)acc_malloc(sizeof(Acc_channel), &acc_live_channels);
    Da os; Da sf;
    channel->outstanding_malloc = da_init(&os, sizeof(void *), NULL);
    channel->spurious_free = da_init(&sf, sizeof(void *), NULL);
    channel->mode = mode;
    return channel;
  }
  else{ //cerr, optional acc_live_channels only tracks channels, not other mallocs/frees
    return channel;
  }
}
void *acc_malloc(size_t size, Acc_channel *channel){
  void *an_allocation_pt = malloc(size);
  if( channel ) da_push((Da *)(channel->outstanding_malloc), &an_allocation_pt);
  return (void *)an_allocation_pt;
}
void acc_free(void *pt, Acc_channel *channel){
  if( channel ){
    void **i = (void **)(((Da *)(channel->outstanding_malloc))->base);
    bool present = false;
    while( i < (void **)(((Da *)(channel->outstanding_malloc))->end) ){
      if( *i == pt ){
        da_pts_nullify((Da *)(channel->outstanding_malloc), i);
        present = true;
      } 
    i++;
    }
    if( present == false ) da_push((Da *)(channel->spurious_free), &pt);
  }
  free(pt);
}
static void count_balance(void *element, void *closure){
  int *counter = (int *)closure;
  if( (void *)element ) (*counter)++;
}
static void acc_rep_helper_BALANCE(Acc_channel *channel){
  int count = 0;
  da_foreach((Da *)channel->outstanding_malloc, count_balance, &count);
  printf("There are %d outstanding allocations.\n", count);
  count = 0;
  da_foreach((Da *)channel->spurious_free, count_balance, &count);
  printf("There are %d spurious frees.\n", count);
}
static void print_pointer(void *element, void *closure){
  if( element ) printf("%d ", *(int *)element);
}
static void acc_rep_helper_FULL(Acc_channel *channel){
  int count = 0;
  da_foreach((Da *)channel->outstanding_malloc, count_balance, &count);
  printf("There are %d outstanding mallocs.\n", count);
  if( count < 10 ){
    printf("The outstanding allocated pointers are: ");
    da_foreach((Da *)channel->outstanding_malloc, print_pointer, NULL);
    printf(".\n");
  }
  count = 0;
  da_foreach((Da *)channel->spurious_free, count_balance, &count);
  printf("There are %d spurious frees.\n", count);
  if( count < 10 ){
    printf("The spuriously freed pointers are: ");
    da_foreach((Da *)channel->outstanding_malloc, print_pointer, NULL);
    printf(".\n");
  }
}
Acc_channel *acc_report(Acc_channel *channel){
  if( channel->mode == acc_NULL ){
    printf("Accounting mode is NULL.");
    return channel;
  }
  if( channel->mode == acc_BALANCE ){
    printf("Accounting mode is BALANCE.\n");
    if( da_emptyq((Da *)(channel->outstanding_malloc)) && da_emptyq((Da *)(channel->spurious_free)) ){
      printf("This channel is in balance.");
    }
    else{
      printf("This channel is out of balance.\n");
      acc_rep_helper_BALANCE(channel);
    }
    return channel;
  }
  if( channel->mode == acc_FULL ){
    printf("Accounting mode is FULL.\n");
    if( da_emptyq((Da *)(channel->outstanding_malloc)) && da_emptyq((Da *)(channel->spurious_free)) ){
      printf("This channel is in balance.");
    }
    else{
      printf("This channel is out of balance.\n");
      acc_rep_helper_FULL(channel);
    }
    return channel;
  }
  if( channel->mode == acc_SELF ){
    printf("Accounting mode is SELF.\n");
    if( da_emptyq((Da *)(channel->outstanding_malloc)) && da_emptyq((Da *)(channel->spurious_free)) ){
      printf("There are no open channels.");
    }
    else {
      printf("The accounting code is out of balance.\n");
      acc_rep_helper_FULL(channel);
    }
    return channel;
  }
}
void acc_close(Acc_channel *channel){
  da_free((Da *)(channel->outstanding_malloc));
  da_free((Da *)(channel->spurious_free));
  if( (channel != &acc_live_channels)
      && (acc_live_channels.mode == acc_SELF) ){
    acc_free(channel, &acc_live_channels);
    return;
  }
  else return;
}

void *crash_and_burn_malloc(size_t size){}
void crash_and_burn_free(void *pt){}


//-----------------------------------------------------------------
//functions definitions for Das

// constructors / destructors
Da *da_init(Da *dap, size_t element_size, Acc_channel *channel){
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
char *da_expand(Da *dap){
  char *old_base = dap->base;
  size_t end_offset = dap->end - old_base;
  size_t new_size = dap->size << 1;
  char *new_base = acc_malloc(new_size, (Acc_channel *)(dap->channel));
  memcpy( new_base, old_base, end_offset + dap->element_size);
  acc_free(old_base, (Acc_channel *)(dap->channel));
  dap->base = new_base;
  dap->end = new_base + end_offset;
  dap->size = new_size;
  return old_base;
}
bool da_boundq(Da *dap){
  return dap->end > (dap->base + dap->size);
}

// status / attributes
//
bool da_emptyq(Da *dap){
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

// accessing
//
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

// iteration, f is given a pointer to an element and a pointer to the closure

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
//would like da_exists and da_all to return pointer to element
//abstract helper for da_exists and da_all
static bool da_quantifier(bool complement, Da *dap, bool pred(void *, void*), void *closure){
  char *pt = dap->base;
  bool result = false;
  while( (complement? !result : result) && (pt != dap->end) ){
    result = pred(pt, closure);
    pt += dap->element_size;
  }
  return result;
}
//∃, OR foreach
bool da_exists(Da *dap, bool pred(void *, void*), void *closure){
  return da_quantifier(true, dap, pred, closure);
}
//∀, AND foreach
bool da_all(Da *dap, bool pred(void *, void*), void *closure){
  return da_quantifier(false, dap, pred, closure);
}

// elements are pointers
//
static bool da_pts_exists_0(void *element, void *test_element){ return element == test_element; }
void *da_pts_exists(Da *dap, void *test_element){
  if( da_exists(dap, da_pts_exists_0, test_element) ) return test_element;
  else return NULL;
}
// elements were allocated, now they will all be freed
static void da_pts_free_all_0(void *pt, void *closure){
  acc_free(*(char **)pt, closure);
}
void da_pts_free_all(Da *dap){
  da_foreach(dap, da_pts_free_all_0, dap->channel);
  da_rewind(dap);
}
// elements are pointers
// ept points at an element, we set *ept to NULL
// we pop all NULLs off the top of the stack
void da_pts_nullify(Da *dap, void **ept){
  if(ept >= (void **)(dap->base) && ept < (void **)(dap->end)){
    *ept = NULL;
  }
  while(
        dap->end > dap->base
        &&
        *(void **)(dap->end - dap->element_size) == NULL){
    da_pop(dap, NULL);
  }
}

// matrices - elements are das 
//need to rename/sed a lot of functions before adding
