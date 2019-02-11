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
#include "dispatch.h"

/*
 returns -1 upon failure

 The wstatus returned from wait() might be either the error we returned when exec failed,
 or the return value from the exec'ed process.  We don't know what the possible return
 values are from exec'ed process might be, so we have no way of multiplexing a unique
 exec error code with it.

 specific dispatch versions can be made for specific functions.
*/


int dispatch(char **argv, char **envp){
  if( !argv || !argv[0] ){
    fprintf(stderr, "argv[0] null on call to dispatch().\n");
    return -1;
  }
  pid_t pid = fork(void);
  if( pid == -1 ){
    fprintf(stderr, "fork() failed in dispatch().\n");
    return -1;
  }
  if( pid == 0 ){ // we are the child
    execvpe(argv[0], argv, envp);
    // exec will only return if it has an error ..
    perror(command);
    return -1;
  }else{ // we are the parent
    int wstatus;
    wait(1, &wstatus, 0);
    if(wstatus)
      return -1;
    else
      return 0;
  }
}
