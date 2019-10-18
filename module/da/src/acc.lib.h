#ifndef ACC_LIB_H
#define ACC_LIB_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define malloc crash_and_burn_malloc
#define free crash_and_burn_free

typedef struct AccChannel_struct AccChannel;
typedef struct Da_struct Da; // Da_struct defined in da.lib.h

enum Mode_enum{acc_NULL = 0, acc_BALANCE = 1, acc_FULL = 2, acc_SELF = 3};//0,1,2,3
typedef enum Mode_enum Mode;

struct AccChannel_struct{
  Da *outstanding_malloc;
  Da *spurious_free;
  Mode mode;
}; //name instances of channels with handles

//function declarations for accounting
  AccChannel *acc_open(AccChannel *channel, Mode mode);//initializes channel structs
  void *acc_malloc(size_t size, AccChannel *channel);//works for things besides Das too
  void acc_free(void *pt, AccChannel *channel);//works for things besides Das too
  AccChannel *acc_report(AccChannel *channel);//reports on channels based on mode
  void acc_close(AccChannel *channel);//frees channel itself

  void *crash_and_burn_malloc(size_t size);//sends error message in case of accidental regular malloc
  void crash_and_burn_free(void *);// sends error message in case of accidental regular free


#endif
