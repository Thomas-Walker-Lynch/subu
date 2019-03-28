/*
Scans a tranche and lists the named target files.

stdout is not explicitly named.  Would be intersting if stdout
were also listed if anything is sent to stdout. Might distinguish
between blank lines being sent and text being sent to stdout.

*/
  
#include <stdio.h>
#include <unistd.h>
#include <da.h>
#include "tranche.lib.h"

#define ERR_ARGC 1
#define ERR_SRC_OPEN 2


int main(int argc, char **argv, char **envp){
  if(argc != 2){
    fprintf(stderr, "usage: %s <source>\n",argv[0]);
    return ERR_ARGC;
  }
  char *src_file_name = argv[1];

  int dep_fd;
  FILE *src_file = fopen(src_file_name, "r");
  unsigned int err = 0;
  if(!src_file){
    fprintf(stderr,"could not open tranche source file %s\n", src_file_name);
    err+= ERR_SRC_OPEN;
  }
  if(err) return err;

  Da targets;
  da_alloc(&targets, sizeof(char *));
  tranche_targets(src_file, &targets);
  da_strings_puts(&targets);
  fclose(src_file);
  return 0;
}
