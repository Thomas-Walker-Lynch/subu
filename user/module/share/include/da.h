#ifndef DA_LIB_H
#define DA_LIB_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  char *base;
  char *end; // one byte/one element off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t element_size;
} Da;

//#include "da_na.lib.h"

#define RETURN(dap, r)                      \
  { da_free_elements(dap); return r; }


void da_alloc(Da *dap, size_t element_size);
void da_free(Da *dap);
void da_rewind(Da *dap);
bool da_empty(Da *dap);
size_t da_length(Da *dap);
void da_rebase(Da *dap, char *old_base, void *pta);
char *da_expand(Da *dap);
bool da_boundq(Da *dap);

char *da_index(Da *dap, size_t i);
char *da_push_alloc(Da *dap);
char *da_push(Da *dap, void *element);
bool da_pop(Da *dap, void *element);

bool da_endq(Da *dap, void *pt);
void da_map(Da *dap, void f(void *, void *), void *closure);
  
void da_free_elements(Da *dap);

void da_ints_print(Da *dap, char *sep);
bool da_integer_repeats(Da *dap);
int da_integer_sum(Da *dap);


void da_strings_print(Da *dap, char *sep);

bool da_strings_exists(Da *string_arrp, char *test_string);
void da_strings_set_insert(Da *string_arrp, char *proffered_string, void destruct(void *));
void da_strings_set_union(Da *string_arrp, Da *proffered_string_arrp, void destruct(void *));


char *da_string_input(Da *dap, FILE *file);
void da_string_push(Da *dap0, char *string);

void da_cat(Da *dap_base, Da *dap_cat);

void da_present(Da **dar, int dar_size, void *closure);
bool da_equal(Da *da_el, Da *test_el);
void da_matrix_map(Da **dar, int dar_size, void f(void *,void*), void *closure);

bool da_exists(Da *dap, bool f(void *, void*), void *closure);
bool da_all(Da *dap, bool f(void *, void*), void *closure);

//matrix functions
void da_erase(Da *damp);
Da *da_push_row_alloc(Da *damp);
Da *da_push_row(Da *damp, Da *dap);
void da_push_column(Da *damp, Da *dap, void *fill);

void da_every_row(Da *damp, void f(void *, void *), void *closure);
Da *da_longer(Da *dap0, Da *dap1);
Da *da_longest(Da *damp);
void da_every_column(Da *damp, void f(void *, void *), void *closure);

Da *da_matrix_transpose(Da *damp, void *fill);

bool da_length_equal(Da *dap0, Da *dap1);
bool da_all_rows_same_length(Da *damp);
bool da_integer_all_rows_repeat(Da *damp);
bool da_integer_all_columns_repeat(Da *damp);
bool da_integer_repeats_matrix(Da *damp);

Da *da_integer_transpose(Da *damp, int *fill);
int *da_integer_to_raw_image_matrix(Da *damp, int fill);
int *da_integer_to_raw_image_transpose(Da *damp, int fill);


/*
Accounting for all the pointers for which we allocated memory on the heap.

Replacement for malloc and free that allows us to check if all the memory we allocated was freed, and if all the memory we tried to free was actually allocated by keeping registers of pointers in Da structs.
 */

#define MALLOC da_malloc_counted
#define FREE da_free_counted
#define ACCOUNT da_start_accounting()
#define BALANCE da_result_accounting()
#define CLOSE_ACC da_na_free(&heap_acc); da_na_free(&extra_frees)

//These variables must be declared at global scope in file that will use them.
extern Da heap_acc; extern Da extra_frees; extern bool accounting;

void da_start_accounting();
void *da_malloc_counted(size_t mem_size);
void da_free_counted(void *pt);
bool da_result_accounting(void);

char *da_na_expand(Da *dap);
char *da_na_push_alloc(Da *dap);
char *da_na_push(Da *dap, void *element);
void da_na_free(Da *dap);


#endif

