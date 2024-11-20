/*

*/

#include <stdio.h>
#include "sss_cache.lib.h"

int main(int argc, char **argv, char **env){
  char *command = argv[0];
  if( argc != 1 ){
    fprintf(stderr, "usage: %s", command);
    return 1;
  }
  return user_mk(argv[1]);
}
