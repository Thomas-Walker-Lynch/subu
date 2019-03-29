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
#include <da.h>
#include "tranche.lib.h"

int main(int argc, char **argv, char **envp){
  if(argc != 2){
    fprintf(stderr, "usage: %s <source-file>\n",argv[0]);
    return TRANCHE_ERR_ARG_PARSE;
  }
  FILE *file = fopen(argv[1], "r");
  if(!file){
    fprintf(stderr,"could not open file %s\n", argv[1]);
    return TRANCHE_ERR_SRC_OPEN;
  }
  Da targets;
  da_alloc(&targets, sizeof(int));
  int fd = STDOUT_FILENO;
  da_push(&targets, &fd);
  tranche_send(file, &targets);
  da_free(&targets);
  fclose(file);
  return 0;
}
