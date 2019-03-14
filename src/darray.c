/*
Dynamic Array

*/

void d_alloc(subudb_subu_element **base, size_t *s, size_t item_size){
  *s = 4 * item_size;
  *base = malloc(*s);
}

// doubles size of an array
void d_expand(void **base, void **pt, size_t *s){
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
bool d_bounds(void *base, void *pt, size_t s){
  return pt >= base + s;
}

void d_push(void **base, void **pt, size_t *s, void *item, size_t item_size){
  while( *pt + item_size >= *base + *s ){
    expand(base, pt, s);
  }
  memcpy(*pt, item, item_size);
  *pt += item_size;
}

// special case when the dynamic array is holding pointers to items on the heap
void d_map(void *base, void *end_pt){
  void *pt = base;
  while( pt != end_pt ){
    free(pt->subuname);
    free(pt->subu_username);
  pt++;
  }
  free(base);  
}
