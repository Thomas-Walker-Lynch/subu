/*
Dynamic Array

Cannot expand an empty array.

Accounting built into code. Alternative version of malloc and free that will invoke accounting for pointers that are allocated
but not freed, or freed but not allocated.

*/

#include "da.lib.h"

//function definitions for accounting
  Acc_channel *acc_open(Acc_channel channel, int mode){}
  void *acc_malloc(size_t size, Acc_channel channel){}
  void acc_free(void *pt, Acc_channel channel){}
  Acc_channel *acc_report(Acc_channel channel){}
  void acc_close(Acc_channel channel){}

  void *crash_and_burn_malloc(size_t size){}
  void crash_and_burn_free(void *){}

//functions definitions for Das

//possible helper functions
static acc_open_NULL(){}
static acc_open_BALANCE(){}
static acc_open_FULL(){}
static acc_open_SELF(){}

static acc_report_NULL(){}
static acc_report_BALANCE(){}
static acc_report_FULL(){}
static acc_report_SELF(){}

static da_init_NULL(){}
static da_init_BALANCE(){}
static da_init_FULL(){}
static da_init_SELF(){}

static da_malloc_NULL(){}
static da_malloc_BALANCE(){}
static da_malloc_FULL(){}
static da_malloc_SELF(){}

static da_free_NULL(){}
static da_free_BALANCE(){}
static da_free_FULL(){}
static da_free_SELF(){}


// constructors / destructors
  Da *da_init(Da *dap, size_t element_size, Acc_channel channel){}
  void da_rewind(Da *dap){}
  void da_rebase(Da *dap, char *old_base, void *pta){}
  char *da_expand(Da *dap){}
  bool da_boundq(Da *dap){}
  void da_erase(Da *dap){}

// status / attributes
//
  bool da_empty(Da *dap){}
  bool da_equal(Da *da_0, Da *da_1){}
  size_t da_length(Da *dap){}
  bool da_length_equal(Da *dap0, Da *dap1){}

// accessing
//
  char *da_index(Da *dap, size_t i){}
  char *da_push(Da *dap, void *element){}
  bool da_pop(Da *dap, void *element){}

// iteration, f is given a pointer to an element and a pointer to the closure
  bool da_endq(Da *dap, void *pt){}
  bool da_right_bound(Da *dap, void *pt){}
  void da_foreach(Da *dap, void f(void *, void *), void *closure){}
  bool da_exists(Da *dap, bool f(void *, void*), void *closure){}
  bool da_all(Da *dap, bool f(void *, void*), void *closure){}

// elements are pointers
//
  void *da_pts_exists(Da *dap, void *test_element){}
  void da_pts_free_all(Da *dap){}
  void da_pts_nullify(Da *dap, void **ept){}

// matrices - elements are das 
//need to rename/sed a lot of functions before adding
