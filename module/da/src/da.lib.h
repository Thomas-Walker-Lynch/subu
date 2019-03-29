#ifndef DA_LIB_H
#define DA_LIB_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct Da{
  char *base;
  char *end; // one byte/one element off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t element_size;
} Da;

#define RETURN(dap, r)                      \
  { da_map(dap, da_free, NULL); return r; }

void da_alloc(Da *dap, size_t element_size);
void da_free(Da *dap);
void da_rewind(Da *dap);
bool da_emptyq(Da *dap);
size_t da_length(Da *dap);
char *da_expand(Da *dap);
void da_rebase(Da *dap, char *old_base, void *pta);
bool da_endq(Da *dap, void *pt);
bool da_boundq(Da *dap);
void da_push(Da *dap, void *element);
bool da_pop(Da *dap, void *element);
void da_cat(Da *dap_base, Da *dap_cat);
void da_push_string(Da *dap0, char *string);
char *da_index(Da *dap, size_t i);
void da_map(Da *dap, void f(void *, void *), void *closure);
void da_free_elements(Da *dap);
void da_strings_puts(Da *dap);
void da_ints_print(Da *dap);
char *da_fgets(Da *dap, FILE *fd);

#endif

