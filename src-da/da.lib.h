#ifndef DA_LIB_H
#define DA_LIB_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct Da{
  char *base;
  char *end; // one byte/one item off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t item_size;
} Da;

#define RETURN(dap, r)                      \
  { da_map(dap, da_free, NULL); return r; }

void da_alloc(Da *dap, size_t item_size);
char *da_expand(Da *dap);
void da_rebase(Da *dap, char *old_base, void *pta);
bool da_endq(Da *dap, void *pt);
bool da_boundq(Da *dap);
void da_push(Da *dap, void *item);
void da_map(Da *dap, void f(void *, void *), void *closure);
void da_free(void *pt, void *closure);

#endif

