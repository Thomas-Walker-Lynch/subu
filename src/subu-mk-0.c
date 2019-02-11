/*
  subu-mk-0 command

*/

#include <stdio.h>
#include "subu-mk-0.fi.h"

int main(int argc, char **argv, char **env){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return ERR_ARG_CNT;
  }
  uid_gid ug = subu-mk-0(argv[1]);
  return ug.error;
}
