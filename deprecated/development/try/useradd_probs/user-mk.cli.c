/*

*/

#include <stdio.h>
#include "user-mk.lib.h"

int main(int argc, char **argv, char **env){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return 1;
  }
  return user_mk(argv[1]);
}
