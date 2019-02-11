/*
fork/exec/wait a command

if the error values returned by the exec'd program
are less than 1 << 16, then 

*/

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "local_common.h"

struct run_err_struct run_err;

int run(char **argv, char **envp){
  run_err_init(run_err);
  if( !argv || !argv[0] ){
    fprintf(stderr, "argv[0] null on call to run().\n");
    run_err.admin = ADMIN_ERR_NO_COMMAND;
    return;
  }
  pid_t pid = fork(void);
  if( pid == -1 ){
    fprintf(stderr, "fork() failed in run().\n");
    run_err.admin = ADMIN_ERR_FORK;
    return;
  }
  if( pid == 0 ){ // we are the child
    int run_err = execvpe(argv[0], argv, envp);
    // exec will only return if it has an error ..
    perror(command);
    run_err.exec = errno;
    return errno;
  }else{ // we are the parent
    int wstatus;
    wait(1, &wstatus, 0);
    if(wstatus != 0) run_err.process = wstatus;
    if( run_err_has(run_err) )
      return -1;
    else
      return 0;
  }
}
