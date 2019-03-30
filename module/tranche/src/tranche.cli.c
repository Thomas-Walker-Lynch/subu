/*
The purpose of this tools is to facilitate putting prototypes (declarations) next
to implementations (definitions) in a single source file of a C/C++ programs. 

Sends code tranches from a single source file into one or more output files.
Scans through the single source file looking for lines of the form:

  #tranche-begin filename filename ...

With the # as the first non-space character on the line, with one or more filenames
following the tag. Upon finding such a line, copies all following lines into the
listed files, until reaching the end marker:

  #tranche-end

Files are opened for create or append.  Typically the user will want to delete
the tranche output files before running tranche a second time.

.. currently tranche_send will probably mess up if the user nests a tranche to
the same file as one already open in the containing tranche ..

.. should allow multiple source file args
*/
  
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <da.h>
#include "tranche.lib.h"

int main(int argc, char **argv, char **envp){
  char *src_file_path = 0;
  char *tdir = 0;  

  { // argument parse
    Da args; // we will queue the non option args here
    Da *argsp = &args;
    da_alloc(argsp, sizeof(char *));

    int err_cnt = 0;
    char **pt = argv;
    char *option;
    char *value; // currently all our tranche options have exactly one value
    while(*pt){
      if( **pt == '-' ){
        option = *pt + 1;
        if(!*option){
          fprintf(stderr, "Currently there is no lone '-' option.\n");
          err_cnt++;
          goto endif;
        }
        pt++; if(!*pt || **pt == '-'){
          fprintf(stderr, "Missing value for option %s.\n", option);
          err_cnt++;
          if(!*pt) break; // then nothing more to parse
          goto endif;
        }
        value = *pt;
        if( !strcmp(option, "tdir") ){
          tdir = value;
          goto endif;
        }
        fprintf(stderr, "Unrecognized option %s.", option);
        err_cnt++;
        goto endif;
      }
      // else clause
      da_push(argsp, pt);
    endif:
    pt++;
    }
    int args_cnt = da_length(argsp);
    if(args_cnt > 1) src_file_path = *(char **)da_index(argsp, 1);

    // arg contracts
    if(args_cnt > 2){
      fprintf(stderr, "too many arguments\n");
      err_cnt++;
    }
    if(err_cnt > 0){
      fprintf(stderr, "usage: %s [<src_file_path>] [-tdir <dir>]\n", argv[0]);
      return TRANCHE_ERR_ARG_PARSE;
    }
    da_free(argsp); // this only frees the array itself, not the things it points to

  }// end of argument parse

  FILE *src_file;
  { //source and mfile open
    if(!src_file_path)
      src_file = stdin;
    else
      src_file = fopen(src_file_path, "r");

    int err = 0;
    if(!src_file){
      fprintf(stderr,"could not open tranche source file %s\n", src_file_path);
      err+= TRANCHE_ERR_SRC_OPEN;
    }
    if(err) return err;
  }

  Da targets;
  da_alloc(&targets, sizeof(int));
  int fd = STDOUT_FILENO;
  da_push(&targets, &fd);
  if(tdir)chdir(tdir);

  tranche_send(src_file, &targets);

  da_free(&targets);
  fclose(src_file);
  return 0;
}
