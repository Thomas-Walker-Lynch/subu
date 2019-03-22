/*
The purpose of this tools is to facilitate putting prototypes (declarations) next
to implementations (definitions) in a single source file of a C/C++ programs. 

Splits a single source file into multiple files.  Scans through the single
source file looking for lines of the form:

  #tranche filename ...

With the # as the first non-space character on the line, and only filename
following the tag. Upon finding such a line, copies all following lines into the
listed files, until reaching the end marker:

  #endtranche

A next improvement of this file would be to support variables to be passed in
for the file names.  As it stands, changing the file name requires editing 
the source file.

*/

#include <da.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//--------------------------------------------------------------------------------
// parsing

char tranche_begin_tag[] = "#tranche";
size_t tranche_begin_tag_len = 8;

char tranche_end_tag[] = "#endtranche";
size_t tranche_end_tag_len = 11;

// given a line
// returns beginning of file name list
static char *is_tranche_begin(char *pt){
  while( *pt && isspace(*pt) ) pt++;
  if(!*pt) return NULL;
  if( strncmp(pt, tranche_begin_tag, tranche_begin_tag_len) ) return NULL;
  return pt + tranche_begin_tag_len;
}

static char *is_tranche_end(char *pt){
  while( *pt && isspace(*pt) ) pt++;
  if(!*pt) return NULL;
  if( strncmp(pt, tranche_end_tag, tranche_end_tag_len) ) return NULL;
  return pt + tranche_end_tag_len;
}

static bool parse_file_list(Da *file_names, char *pt0){
  char *pt1;
  while( *pt0 && isspace(*pt0) ) pt0++;
  while( *pt0 ){
    pt1 = pt0;
    while( *pt1 && !isspace(pt1) ) pt1++;
    char *file_name = strndup(pt0, pt1 - pt0);
    da_push(file_names, file_name);
    pt0 = pt1;
    while( *pt0 && isspace(*pt0) ) pt0++;
  }
}



//--------------------------------------------------------------------------------
// da_map calls

static void tranche_open_fd(void *fn, void *closure){
  char *file_name = (char *)fn;
  Da *fds = (Da *)closure;
  int fd = open(fn, O_CREAT | O_APPEND);
  if(fd == -1){
    fprintf(stderr, "Could not open file %s\n", file_name);
    return;
  }
  da_push(fds, &fd);  
  return;
}
static void tranche_open_fds(Da *fns, Da *fds){
  da_map(fns, tranche_open_fd, fds);
}

static void tranche_close_fd(void *fd, void *closure){
  close(*(int *)fd);
}
static void tranche_close_fds(Da *fds){
  da_map(fds, tranche_close_fd, NULL);
}

static void tranche_puts(void *fd, void *string){
  write(*(int *)fd, string, strlen(string));
}
static void tranche_puts_all(Da *fds, char *string){
  da_map(fds, tranche_puts, string);
}

//--------------------------------------------------------------------------------
// 

int tranche_send(FILE *src, Da *arg_fds){
  char *pt;
  Da line;
  Da file_names;
  Da fds;
  da_alloc(&line, sizeof(char));
  da_alloc(&file_names, sizeof(char *));
  da_alloc(&fds, sizeof(int));
  
  da_fgets(&line, src);
  while( !feof(src) && !is_tranche_end(line.base) ){
    pt = is_tranche_begin(line.base);
    if(pt){ // then this line is the start of a nested tranche block
      parse_file_list(&file_names, pt);
      tranche_open_fds(&file_names, &fds);
      da_free_elements(&file_names);
      tranche_send(src, &fds);
      tranche_close_fds(&fds);
    }else{
      tranche_puts_all(arg_fds, line.base);
      da_rewind(&line);
      da_fgets(&line, src);
    }
  }
  
  da_free(&line);
  da_free(&file_names);
  da_free(&fds);
  return 0;
}
