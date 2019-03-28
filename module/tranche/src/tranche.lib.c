/*
The purpose of this tools is to facilitate putting prototypes (declarations) next
to implementations (definitions) in a single source file of a C/C++ programs. 

Splits a single source file into multiple files.  Scans through the single
source file looking for lines of the form:

  #tranche-begin filename ...

With the # as the first non-space character on the line, and only filename
following the tag. Upon finding such a line, copies all following lines into the
listed files, until reaching the end marker:

  #tranche-end

A next improvement of this file would be to support variables to be passed in
for the file names.  As it stands, changing the file name requires editing 
the source file.

Files are opened for create or append, so opening to the same file will append
to the end.

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

char newline = '\n';
char terminator = 0;

char tranche_begin_tag[] = "#tranche";
size_t tranche_begin_tag_len = 8;

char tranche_end_tag[] = "#tranche-end";
size_t tranche_end_tag_len = 12;

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
  pt1 = pt0;
  while( *pt0 ){
    while( *pt1 && !isspace(*pt1) ) pt1++;
    char *file_name = strndup(pt0, pt1 - pt0);
    da_push(file_names, &file_name);
    while( *pt1 && isspace(*pt1) ) pt1++;
    pt0 = pt1;
  }
}



//--------------------------------------------------------------------------------
// da_map calls

static void tranche_open_fd(void *fnp, void *closure){
  char *file_name = *(char **)fnp;
  Da *fdap = (Da *)closure;
  int fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
  if(fd == -1){
    fprintf(stderr, "Could not open file %s\n", file_name);
    return;
  }
  da_push(fdap, &fd);  
  return;
}
static void tranche_open_fds(Da *fnap, Da *fdap){
  da_map(fnap, tranche_open_fd, fdap);
}

static void tranche_close_fd(void *fdp, void *closure){
  close(*(int *)fdp);
}
static void tranche_close_fds(Da *fdap){
  da_map(fdap, tranche_close_fd, NULL);
  da_rewind(fdap);
}

static void tranche_puts(void *fdp, void *string){
  write(*(int *)fdp, string, strlen(string));
}
static void tranche_puts_all(Da *fdap, char *string){
  da_map(fdap, tranche_puts, string);
}



//--------------------------------------------------------------------------------
// we have a little problem if the user tries to tranche two things to the same file ..
int tranche_send(FILE *src, Da *arg_fdap){
  char *pt;
  Da line;
  Da file_name_arr;
  Da fda;
  da_alloc(&line, sizeof(char));
  da_alloc(&file_name_arr, sizeof(char *));
  da_alloc(&fda, sizeof(int));

  while( !feof(src) ){
    da_fgets(&line, src);
    if( is_tranche_end(line.base) ) break;
    pt = is_tranche_begin(line.base);
    if(pt){ // then this line is the start of a nested tranche block
      parse_file_list(&file_name_arr, pt);
      tranche_open_fds(&file_name_arr, &fda);
      da_free_elements(&file_name_arr);
      tranche_send(src, &fda);
      tranche_close_fds(&fda);
    }else{
      da_pop(&line, NULL); // pop the terminating zero
      da_push(&line, &newline);
      da_push(&line, &terminator);
      tranche_puts_all(arg_fdap, line.base);
    }
    da_rewind(&line);
  }

  da_free(&line);
  da_free(&file_name_arr);
  da_free(&fda);
  return 0;
}
