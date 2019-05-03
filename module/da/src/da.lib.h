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

#define RETURN(dap, r)                      \
  { da_pts_free_all(dap); return r; }


// constructors / destructors
//
  void da_alloc(Da *dap, size_t element_size);
  void da_free(Da *dap);
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
  void da_map(Da *dap, void f(void *, void *), void *closure); // this is really 'foreach' to be renamed
  bool da_exists(Da *dap, bool f(void *, void*), void *closure);
  bool da_all(Da *dap, bool f(void *, void*), void *closure);

// elements are pointers
// would be better if exists returned NULL or a pointer to the existing element-
  bool da_pts_exists(Da *dap, void *test_element);
  void da_pts_free_all(Da *dap); // calls free on all elements
  void da_pts_nullify(Da *dap, void **ept); // sets *ept to NULL, pops all NULLs from top of stack

// elements are integers
  void da_ints_print(Da *dap, char *sep);
  bool da_integer_repeats(Da *dap);
  int da_integer_sum(Da *dap);

// the array itself is a string
// careful if you add a null terminator it will become part of the da_string, affects iteration etc.
  char *da_string_input(Da *dap, FILE *file);
  void da_string_push(Da *dap0, char *string);
  void da_cat(Da *dap_base, Da *dap_cat);

// when the array holds pointers to C strings
  void da_strings_print(Da *dap, char *sep);
  bool da_strings_exists(Da *string_arrp, char *test_string);
  void da_strings_set_insert(Da *string_arrp, char *proffered_string, void destruct(void *));
  void da_strings_set_union(Da *string_arrp, Da *proffered_string_arrp, void destruct(void *));

// matrix
// There is a top level master da array.  Its elements are da arrays, these are said to be rows
  void da_mat_map(Da **dar, int dar_size, void f(void *,void*), void *closure);

  Da *da_mat_push_row_alloc(Da *damp);
  Da *da_mat_push_row(Da *damp, Da *dap);
  void da_mat_push_column(Da *damp, Da *dap, void *fill);

  void da_mat_every_row(Da *damp, void f(void *, void *), void *closure);
  Da *da_mat_longer(Da *dap0, Da *dap1);
  Da *da_mat_longest(Da *damp);
  void da_mat_every_column(Da *damp, void f(void *, void *), void *closure);

  Da *da_mat_transpose(Da *damp, void *fill);

  bool da_mat_all_rows_same_length(Da *damp);
  bool da_mat_ints_all_rows_repeat(Da *damp);
  bool da_mat_ints_all_columns_repeat(Da *damp);
  bool da_mat_ints_repeats_matrix(Da *damp);

  Da *da_mat_ints_transpose(Da *damp, int *fill);
  int *da_mat_ints_to_raw_image_matrix(Da *damp, int fill);
  int *da_mat_ints_to_raw_image_transpose(Da *damp, int fill);

#endif

