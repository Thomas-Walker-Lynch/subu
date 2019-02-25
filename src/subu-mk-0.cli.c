/*
  subu-mk-0 command

*/
#include "subu-mk-0.lib.h"
#include <stdio.h>

int main(int argc, char **argv){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return ERR_SUBU_MK_0_ARG_CNT;
  }
  char *subuname = argv[1];

  return subu_mk_0(subuname, config_file);
}
