/*
dynamic memory accounting

*/


#include "da.lib.h"
#include "acc.lib.h"

#undef malloc
#undef free

//-----------------------------------------------------------------
//function definitions for accounting

AccChannel *acc_open(AccChannel *channel, Mode mode){//acc init
    Da os; Da sf;
    channel->outstanding_malloc = da_init(&os, sizeof(void *), NULL);
    channel->spurious_free = da_init(&sf, sizeof(void *), NULL);
    channel->mode = mode;
    return channel;
}
void *acc_malloc(size_t size, AccChannel *channel){
  void *an_allocation_pt = malloc(size);
  if( channel ) da_push((Da *)(channel->outstanding_malloc), &an_allocation_pt);//tried taking out reference but fixed nothing
  return (void *)an_allocation_pt;
}
void acc_free(void *pt, AccChannel *channel){
  if( channel ){
    void **i = (void **)(((Da *)(channel->outstanding_malloc))->base);
    bool present = false;
    while( i < (void **)(((Da *)(channel->outstanding_malloc))->end) ){
      if( *i == pt ){//this is just da_exists, make it return a pointer
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
  if( element ) (*counter)++;
}
static void acc_rep_helper_BALANCE(AccChannel *channel){
  int count = 0;
  da_foreach((Da *)channel->outstanding_malloc, count_balance, &count);
  printf("There are %d outstanding allocations.\n", count);
  int count1 = 0;
  da_foreach((Da *)channel->spurious_free, count_balance, &count1);
  printf("There are %d spurious frees.\n", count);
}
static void print_pointer(void *element, void *closure){
  if( element ) printf("%d ", *(int *)element);
}
static void acc_rep_helper_FULL(AccChannel *channel){
  printf("There are %d outstanding mallocs.\n", (int)da_length((Da *)(channel->outstanding_malloc)));
  printf("There are %d spurious frees.\n", (int)da_length((Da *)(channel->spurious_free)));
  /*if( 0 < count && count < 10 ){
    printf("The outstanding allocated pointers are: ");
    da_foreach((Da *)channel->outstanding_malloc, print_pointer, NULL);
    printf(".\n");
  }
  if( 0 < count && count < 10 ){
    printf("The spuriously freed pointers are: ");
    da_foreach((Da *)channel->outstanding_malloc, print_pointer, NULL);
    printf(".\n");
    }*/
}
AccChannel *acc_report(AccChannel *channel){
  if( channel->mode == acc_NULL ){
    printf("Accounting mode is NULL.\n");
    return channel;
  }
  if( channel->mode == acc_BALANCE ){
    printf("Accounting mode is BALANCE.\n");
    if( da_is_empty((Da *)(channel->outstanding_malloc)) && da_is_empty((Da *)(channel->spurious_free)) ){
      printf("This channel is in balance.\n");
    }
    else{
      printf("This channel is out of balance.\n");
      acc_rep_helper_BALANCE(channel);
    }
    return channel;
  }
  if( channel->mode == acc_FULL ){
    printf("Accounting mode is FULL.\n");
    if( false ){
      printf("This channel is in balance.\n");
    }
    else{
      printf("This channel is out of balance.\n");
      acc_rep_helper_FULL(channel);
    }
    return channel;
  }
  if( channel->mode == acc_SELF ){
    printf("Accounting mode is SELF.\n");
    if( true/*da_is_empty((Da *)(channel->outstanding_malloc)) && da_is_empty((Da *)(channel->spurious_free)) */){
      printf("There are no open channels.\n");
    }
    else {
      printf("The accounting code is out of balance.\n");
      acc_rep_helper_FULL(channel);
    }
    return channel;
  }
}
void acc_close(AccChannel *channel){
  da_free((Da *)(channel->outstanding_malloc));
  da_free((Da *)(channel->spurious_free));
  return;
}

void *crash_and_burn_malloc(size_t size){}
void crash_and_burn_free(void *pt){}

