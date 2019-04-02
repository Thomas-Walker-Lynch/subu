#tranche subu-bind.cli.c
/*
mount a subu user directory into master's subuland
uses unmount to undo this

*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){

  if( argc != 4){
    fprintf(stderr, "usage: %s masteru subu_username subuhome\n",argv[0]);
    return SUBU_ERR_ARG_CNT;
  }

  int rc;
  char *mess;
  rc = subu_bind(&mess, argv[1], argv[2], argv[3]);
  if(rc != 0){
    fprintf(stderr, "subu-bind: %s\n", mess);
    return rc;
  }
  return 0;
}
#tranche-end
