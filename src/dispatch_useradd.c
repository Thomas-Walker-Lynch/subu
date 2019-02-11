/*
There is no C library interface to useradd(8), but if there were, these functions
would be found there instead.

*/

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "local_common.h"
#include "dispatch_useradd.h"

// we have a contract with the caller that argv[1] is always the subuname
useradd_ret dispatch_useradd(char **argv, char **envp){
  run_err_init(run_err);
  if( !argv || !argv[0] || !argv[1]){
    fprintf(stderr,"useradd() needs a first argument as the name of the user to be made");
    return useradd_ret(USERADD_ERR_ARGC, NULL);
  }
  char *subu_name = argv[1];
  if( run(argv, envp) == -1 ){
    fprintf(stderr,"%s failed\n", argv[0]);
    return useradd_ret(USER_ERR_RUN);
  }
  struct password *pw_record = getpwnam(subu_name);
  uint count = 1;
  while( !pw_record && count <= 3 ){
    #ifdef DEBUG
    printf("try %u, getpwnam failed, trying again\n", count);
    #endif
    sleep(1);
    pw_record = getpwnam(subu_name);
    count++;
  }
  if( !pw_record ){
    return struct useradd_ret(USERADD_ERR_PWREC, NULL);
  }
  return struct useradd_ret(0, pw_record);
}

