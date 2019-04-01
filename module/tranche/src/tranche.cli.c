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

*/
  
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <da.h>
#include "tranche.lib.h"

int main(int argc, char **argv, char **envp){

  int err = 0;
  char *tdir = 0;  
  Da args; // we will queue the non option args here
  Da *argsp = &args; // collection of the non-option non-option-value args
  da_alloc(argsp, sizeof(char *));
  { // argument parse
    char **pt = argv + 1; // skip the command name
    char *option;
    char *value; // currently all our tranche options have exactly one value
    while(*pt){
      if( **pt == '-' ){
        option = *pt + 1;

        // options that take no values:
        if(!*option){
          fprintf(stderr, "Currently there is no lone '-' option.\n");
          err |= TRANCHE_ERR_ARG_PARSE;
          goto endif;
        }
        if( !strcmp(option, "h") || !strcmp(option, "help") ){
          err |= TRANCHE_ERR_HELP; // this will force the usage message, though it will also return an error
          goto endif;
        }

        // options that take one value:
        pt++; if(!*pt || **pt == '-'){
          fprintf(stderr, "Missing value for option %s.\n", option);
          err |= TRANCHE_ERR_ARG_PARSE;
          if(!*pt) break; // then nothing more to parse
          goto endif;
        }
        value = *pt;
        if( !strcmp(option, "tdir") ){
          tdir = value;
          goto endif;
        }
        fprintf(stderr, "Unrecognized option %s.", option);
        err |= TRANCHE_ERR_ARG_PARSE;
        goto endif;
      }
      // else clause
      da_push(argsp, pt);
    endif:
    pt++;
    }
    if(err){
      fprintf(stderr, "usage: %s [<src_file_path>].. [-tdir <dir>]\n", argv[0]);
      return err;
    }
  }// end of argument parse

  Da targets;
  Da *target_arrp = &targets;
  da_alloc(target_arrp, sizeof(int));
  int fd = STDOUT_FILENO;
  da_push(target_arrp, &fd);
  if(tdir)chdir(tdir);

  Da src_arr;
  Da *src_arrp = &src_arr;
  da_alloc(src_arrp, sizeof(char *));
  da_strings_set_union(src_arrp, argsp, NULL);
  da_free(argsp);

  char *src_file_path;
  FILE *src_file;
  if(da_emptyq(src_arrp))
    tranche_send(stdin, target_arrp);
  else{
    char *pt = src_arrp->base;
    while( pt < src_arrp->end ){
      src_file_path = *(char **)pt;
      src_file = fopen(src_file_path, "r");
      if(!src_file){
        fprintf(stderr,"Could not open source file %s.\n", src_file_path);
        err |= TRANCHE_ERR_SRC_OPEN;
      }else{
        tranche_send(src_file, target_arrp);
        if( fclose(src_file) == -1 ){perror(NULL); err |= TRANCHE_ERR_FCLOSE;}
      }
    pt += src_arrp->element_size;
    }
  }

  da_free(target_arrp);
  da_free(src_arrp);
  return err;
}
