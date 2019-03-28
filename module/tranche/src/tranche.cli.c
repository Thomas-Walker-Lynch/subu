
#include <stdio.h>
#include <unistd.h>
#include <da.h>
#include "tranche.lib.h"

int main(int argc, char **argv, char **envp){
  if(argc != 2){
    fprintf(stderr, "usage: %s <source-file>\n",argv[0]);
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  if(!file){
    fprintf(stderr,"could not open file %s\n", argv[1]);
    return 2;
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
