/*
  subu-mk-0 command

*/

#include <stdio.h>
#include "subu-mk-0.lib.h"

int main(int argc, char **argv, char **env){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return ERR_ARG_CNT;
  }
  return subu_mk_0(argv[1]);
}
