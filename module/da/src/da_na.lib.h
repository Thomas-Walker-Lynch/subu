#ifndef DA_NA_LIB_H
#define DA_NA_LIB_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/*
Alternative versions of the da array constructors and destructors for avoiding the accounting
versions of malloc and free.  We use these in the accounting code itself.

Note the accounting software does not use int, string, strings, or matrix da methods, so those
have not been replaced.

Why are we afraid to let accounting account for itself?  Well otherwise there is a possibility
of a hickup where an array in the accounting code expands while testing a block
of code.

*/

void da_na_alloc(Da *dap, size_t element_size);
void da_na_free(Da *dap);
char *da_na_expand(Da *dap);
char *da_na_push_alloc(Da *dap);
char *da_na_push(Da *dap, void *element);
void da_na_pts_free_all(Da *dap);

#endif
