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
#include "dispatch_exec.lib.h"
#include "dispatch_useradd.lib.h"

// we have a contract with the caller that argv[1] is always the subuname
struct dispatch_useradd_ret_t dispatch_useradd(char **argv, char **envp){
  struct dispatch_useradd_ret_t ret;
  {
    if( !argv || !argv[0] || !argv[1]){
      fprintf(stderr,"useradd() needs a first argument as the name of the user to be made");
      ret.error = DISPATCH_USERADD_ERR_ARGC;
      ret.pw_record = NULL;
      return ret;
    }

    char *subu_name;
    {
      subu_name = argv[1];
      if( dispatch_exec(argv, envp) == -1 ){
        fprintf(stderr,"%s failed\n", argv[0]);
        ret.error = DISPATCH_USERADD_ERR_DISPATCH;
        ret.pw_record = NULL;
        return ret;
      }}

    {
      struct passwd *pw_record = getpwnam(subu_name);
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
        ret.error = DISPATCH_USERADD_ERR_PWREC;
        ret.pw_record = NULL;
        return ret;
      }
      ret.error = 0;
      ret.pw_record = pw_record;
      return ret;
    }}}

