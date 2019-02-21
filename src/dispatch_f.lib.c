/*
 changes the uid, gid, and forks and calls the function
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
#include "dispatch_f.lib.h"

int dispatch_f(char *fname, int (*f)(void *arg), void *f_arg){
  char *perror_src = "dispatch_f_as";
  #ifdef DEBUG
  dbprintf("%s %s\n", perror_src, fname);
  #endif
  pid_t pid = fork();
  if( pid == -1 ){
    perror(perror_src);
    fprintf(stderr, "%s %s\n", perror_src, fname);
    return ERR_FORK;
  }
  if( pid == 0 ){ // we are the child
    int status = (*f)(f_arg);
    exit(status);
  }else{ // we are the parent
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return wstatus;
  }
}

int dispatch_f_euid_egid(char *fname, int (*f)(void *arg), void *f_arg, uid_t euid, gid_t egid){
  char *perror_src = "dispatch_f_as";
  #ifdef DEBUG
  dbprintf("%s %s %u %u\n", perror_src, fname, euid, egid);
  #endif
  pid_t pid = fork();
  if( pid == -1 ){
    perror(perror_src);
    fprintf(stderr, "%s %s %u %u\n", perror_src, fname, euid, egid);
    return ERR_FORK;
  }
  if( pid == 0 ){ // we are the child
    if( seteuid(euid) == -1 ){
      perror(perror_src);
      fprintf(stderr, "%s %s %u %u\n", perror_src, fname, euid, egid);
      return ERR_SETEUID;
    }
    if( setegid(egid) == -1 ){
      perror(perror_src);
      fprintf(stderr, "%s %s %u %u\n", perror_src, fname, euid, egid);
      return ERR_SETEGID;
    }
    int status = (*f)(f_arg);
    exit(status);
  }else{ // we are the parent
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return wstatus;
  }
}


