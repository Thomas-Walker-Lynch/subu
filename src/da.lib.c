/*
Dynamic Array

*/

#include "da.lib.h"

#if INTERFACE
#include<stdlib.h>
#include<stdbool.h>
#endif
#include<string.h>

//--------------------------------------------------------------------------------
// generic

// s is the size of the array in bytes
void da_alloc(void **base, size_t *s, size_t item_size){
  *s = 4 * item_size;
  *base = malloc(*s);
}

// doubles size of an array
void da_expand(void **base, void **pt, size_t *s){
  size_t offset = ((unsigned char *)*pt - (unsigned char *)*base);
  size_t new_s = *s << 1;
  void *new_base = malloc( new_s );
  memcpy( new_base, *base, offset + 1);
  free(base);
  *base = new_base;
  *pt = new_base + offset;
  *s = new_s;
}

// true when pt has run off the end of the area currently allocated for the array
bool da_bound(void *base, void *pt, size_t s){
  return pt >= base + s;
}

void da_push(void **base, void **pt, size_t *s, void *item, size_t item_size){
  while( *pt + item_size >= *base + *s ){
    da_expand(base, pt, s);
  }
  memcpy(*pt, item, item_size);
  *pt += item_size;
}

void da_map(void *base, void *end_pt, void f(void *), size_t item_size){
  void *pt = base;
  while( pt != end_pt ){
    f(pt);
  pt += item_size;
  }
}

//--------------------------------------------------------------------------------
// dynamic array of pointers to strings

// s is still the length of the array in bytes
void daps_alloc(char **base, size_t *s){
  da_alloc((void **)base, s, sizeof(char *));
}

void daps_expand(char **base, char **pt, size_t *s){
  da_expand((void **)base, (void **)pt, s);
}

bool daps_bound(char **base, char **pt, size_t s){
  return da_bound( (void *) base, (void *)pt, s);
}

void daps_push(char **base, char **pt, size_t *s, char *item){
  da_push((void **)base, (void **)pt, s, (void *)item, sizeof(char *));
}

void daps_map(char **base, char **end_pt, void f(void *)){
  da_map((void *)base, (void *)end_pt, f, sizeof(char *));
}

// one use for an array of string pointers is to keep list of
// strings that must be freed.  I.e. 'managed' strings
#if INTERFACE

#define MK_MRS \
  char **mrs;  \
  char **mrs_end; \
  size_t mrs_size; \
  daps_alloc(mrs, &mrs_size);\
  mrs_end = mrs;

#define RETURN(rc) \
  { daps_map(mrs, mrs_end, free); return rc; }

#endif
  
