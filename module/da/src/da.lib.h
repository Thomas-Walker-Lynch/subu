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
void da_rebase(Da *dap, char *old_base, void *pta);
char *da_expand(Da *dap);
bool da_boundq(Da *dap);

char *da_index(Da *dap, size_t i);
void da_push(Da *dap, void *element);
bool da_pop(Da *dap, void *element);

bool da_endq(Da *dap, void *pt);
void da_map(Da *dap, void f(void *, void *), void *closure);

void da_free_elements(Da *dap);

void da_ints_print(Da *dap, char *sep);

void da_strings_print(Da *dap, char *sep);
bool da_strings_exists(Da *string_arrp, char *test_string);
void da_strings_set_insert(Da *string_arrp, char *proffered_string, void destruct(void *));
void da_strings_set_union(Da *string_arrp, Da *proffered_string_arrp, void destruct(void *));


char *da_string_input(Da *dap, FILE *file);
void da_string_push(Da *dap0, char *string);

void da_cat(Da *dap_base, Da *dap_cat);

#endif

