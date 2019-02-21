/*
 fork/execs/wait the command passed in argv[0];
 Returns -1 upon failure.

 The wstatus returned from wait() might be either the error returned by exec
 when it failed, or the return value from the command.  An arbitary command is
 passed in, so we don't know what its return values might be. Consquently, we
 have no way of multiplexing a unique exec error code with the command return
 value within wstatus.  If the prorgrammer knows the return values of the
 command passed in, and wants better behavior, he or she can spin a special
 version of dispatch for that command.
*/

// without this #define execvpe is undefined
#define _GNU_SOURCE   
#include <sys/types.h>
#include <unistd.h>

#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "local_common.h"
#include "dispatch_exec.lib.h"

int dispatch_exec(char **argv, char **envp){
  if( !argv || !argv[0] ){
    fprintf(stderr, "argv[0] null. Null command passed into dispatch().\n");
    return -1;
  }
  #ifdef DEBUG
    dbprintf("dispatching exec:");
    char **apt = argv;
    while( apt ){
      dbprintf(" %s",*apt);
    apt++;
    }
    dbprintf("\n");
  #endif
  char *command = argv[0];
  pid_t pid = fork();
  if( pid == -1 ){
    fprintf(stderr, "fork() failed in dispatch().\n");
    return -1;
  }
  if( pid == 0 ){ // we are the child
    execvpe(command, argv, envp);
    // exec will only return if it has an error ..
    perror(command);
    return -1;
  }else{ // we are the parent
    int wstatus;
    waitpid(pid, &wstatus, 0);
    if(wstatus)
      return -1;
    else
      return 0;
  }
}
