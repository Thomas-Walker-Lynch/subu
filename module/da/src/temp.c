#include <stdlib.h>


typedef struct {
  char *base;
  char *end; // one byte/one element off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t element_size;
} Da;


Da heap_count = {
  .element_size = sizeof(void *),
  .size = 4*sizeof(void *),
  .base = malloc(4*sizeof(void *)),
  .end = this.base
};
void *da_malloc_counted(size_t mem_size);
void da_free_counted(void *pt);

