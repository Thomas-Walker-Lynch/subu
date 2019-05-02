/*
Accounting for all the pointers for which we allocated memory on the heap.

Replacement for malloc and free that allows us to check if all the memory we allocated was freed, and if all the memory we tried to free was actually allocated by keeping registers of pointers in Da structs.
 */

#ifndef DA_NA_LIB_H
#define DA_NA_LIB_H

#define MALLOC da_malloc_counted
#define FREE da_free_counted
#define ACCOUNT Da heap_acc; \
  Da extra_frees;            \
  bool accounting = false;                                    \
  da_start_accounting(heap_acc, extra_frees, accounting);
#define BALANCE da_result_accounting()
#define CLOSE_ACC da_na_free(&heap_acc); da_na_free(&extra_frees);

Da heap_acc; Da extra_frees;
void da_start_accounting(Da heap_acc, Da extra_frees, bool acc);
void *da_malloc_counted(size_t mem_size);
void da_free_counted(void *pt);
bool da_result_accounting(void);

char *da_na_expand(Da *dap);
char *da_na_push_alloc(Da *dap);
char *da_na_push(Da *dap, void *element);
void da_na_free(Da *dap);

#endif
