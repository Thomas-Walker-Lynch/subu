#ifndef DA_LIB_H
#define DA_LIB_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define malloc crash_and_burn_malloc
#define free crash_and_burn_free

typedef struct {
  Da outstanding_malloc;
  Da spurious_free;
  int mode;
} Acc_channel; //name instances of channels with handles

typedef struct {
  char *base;
  char *end; // one byte/one element off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t element_size;
  Acc_channel *channel;//assign during init, set to NULL during free
} Da;

enum Mode{acc_NULL, acc_BALANCE, acc_FULL, acc_SELF};//0,1,2,3

extern Acc_channel acc_live_channels;//acc_NULL or acc_SELF to track acc channels or not, other options return invalid upon report

/*
  Explanation of process for using accounting code.
*/
//function declarations for accounting
  Acc_channel *acc_open(Acc_channel channel, int mode);//initializes channel structs
  void *acc_malloc(size_t size, Acc_channel channel);//works for things besides Das too
  void acc_free(void *pt, Acc_channel channel);//works for things besides Das too
  Acc_channel *acc_report(Acc_channel channel);//reports on channels based on mode
  void acc_close(Acc_channel channel);//frees channel itself

  void *crash_and_burn_malloc(size_t size);//sends error message in case of accidental regular malloc
  void crash_and_burn_free(void *);// sends error message in case of accidental regular free

//function declarations for Das

// constructors / destructors
  Da *da_init(Da *dap, size_t element_size, Acc_channel channel);//calls da_malloc for base pointer
  
  void da_rewind(Da *dap);
  void da_rebase(Da *dap, char *old_base, void *pta);
  char *da_expand(Da *dap);
  bool da_boundq(Da *dap);
  void da_erase(Da *dap);

// status / attributes
//
  bool da_empty(Da *dap);
  bool da_equal(Da *da_0, Da *da_1);
  size_t da_length(Da *dap);
  bool da_length_equal(Da *dap0, Da *dap1);

// accessing
//
  char *da_index(Da *dap, size_t i);
  char *da_push(Da *dap, void *element);
  bool da_pop(Da *dap, void *element);

// iteration,  f is given a pointer to an element and a pointer to the closure
  bool da_endq(Da *dap, void *pt);
  bool da_right_bound(Da *dap, void *pt);
  void da_foreach(Da *dap, void f(void *, void *), void *closure); //used to be da_map
  bool da_exists(Da *dap, bool f(void *, void*), void *closure);
  bool da_all(Da *dap, bool f(void *, void*), void *closure);

// elements are pointers
//
  void *da_pts_exists(Da *dap, void *test_element);
  void da_pts_free_all(Da *dap); // calls free on all elements
  void da_pts_nullify(Da *dap, void **ept); // sets *ept to NULL, pops all NULLs from top of stack


// matrices - elements are das 
//need to rename/sed a lot of functions before adding



#endif

