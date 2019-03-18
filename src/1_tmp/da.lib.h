/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <stdlib.h>
#include <stdbool.h>
void daps_map(char **base,char **end_pt,void f(void *));
#define RETURN(rc) \
  { daps_map(mrs, mrs_end, free); return rc; }
void daps_alloc(char **base,size_t *s);
#define MK_MRS \
  char **mrs;  \
  char **mrs_end; \
  size_t mrs_size; \
  daps_alloc(mrs, &mrs_size);\
  mrs_end = mrs;
void daps_push(char **base,char **pt,size_t *s,char *item);
bool daps_bound(char **base,char **pt,size_t s);
void daps_expand(char **base,char **pt,size_t *s);
void da_map(void *base,void *end_pt,void f(void *),size_t item_size);
void da_push(void **base,void **pt,size_t *s,void *item,size_t item_size);
bool da_bound(void *base,void *pt,size_t s);
void da_expand(void **base,void **pt,size_t *s);
void da_alloc(void **base,size_t *s,size_t item_size);
#define INTERFACE 0
