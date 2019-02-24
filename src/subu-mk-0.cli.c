/*
  subu-mk-0 command

*/
#include "subu-mk-0.lib.h"
#include <stdio.h>

//  char *config_file = "/etc/subu.db";
char config_file[] = "subu.db";

int main(int argc, char **argv, char **envp){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return ERR_SUBU_MK_0_ARG_CNT;
  }
  char *subuname = argv[1];

  return subu_mk_0(subuname, config_file);
}
