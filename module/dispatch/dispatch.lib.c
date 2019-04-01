

#include "dispatch.h"

// we need the declaration for uid_t etc.
// without this #define execvpe is undefined
#define _GNU_SOURCE   

#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

void dispatch_f_mess(char *fname, int err, char *dispatchee){
  if(err == 0) return;
  fprintf(stderr, "%s: ", fname); // if fprintf gets an error, errno will be overwritten
  if(err > ERR_DISPATCH){
    fprintf(stderr, "dispatchee \"%s\" returned the error %d\n", dispatchee, err);
    return;
  }
  switch(err){
  case ERR_DISPATCH_NEGATIVE_RETURN_STATUS:
    fprintf(stderr, " dispatchee \"%s\" returned a negative status.", dispatchee);
    break;
  case ERR_DISPATCH_F_FORK:
  case ERR_DISPATCH_F_SETEUID:
  case ERR_DISPATCH_F_SETEGID:
    fputc(' ', stderr);
    perror(dispatchee);
    break;
  case ERR_DISPATCH_NULL_EXECUTABLE:
    fprintf(stderr, " executable was not specified");
    break;
  case ERR_DISPATCH_EXEC:
    // exec is running in another process when it fails, so we can't see the errno value it set
    fprintf(stderr, " exec of \"%s\" failed", dispatchee);
    break;
  default:
    fprintf(stderr, " returned undefined status when dispatching \"%s\"", dispatchee);
  }
  fputc('\n', stderr);
}

//--------------------------------------------------------------------------------
// interface call point, dispatch a function
int dispatch_f(char *fname, int (*f)(void *arg), void *f_arg){
  #ifdef DEBUG
  dbprintf("%s %s\n", "dispatch_f", fname);
  #endif
  pid_t pid = fork();
  if( pid == -1 ) return ERR_DISPATCH_F_FORK; // something went wrong and we are still in the parent process
  if( pid == 0 ){ // we are the child
    int status = (*f)(f_arg); // we require that f return a zero or positive value
    if( status < ERR_DISPATCH ) status = ERR_DISPATCH_NEGATIVE_RETURN_STATUS;
    exit(status);
  }else{ // we are the parent
    int err;
    waitpid(pid, &err, 0);
    return err;
  }
}

//--------------------------------------------------------------------------------
// interface call point, dispatch a function with a given euid/egid
// of course this will only work if our euid is root in the first place
int dispatch_f_euid_egid(char *fname, int (*f)(void *arg), void *f_arg, uid_t euid, gid_t egid){
  #ifdef DEBUG
  dbprintf("%s %s as euid:%u egid:%u\n", "dispatch_f_euid_egid", fname, euid, egid);
  #endif
  pid_t pid = fork();
  if( pid == -1 ) return ERR_DISPATCH_F_FORK;
  if( pid == 0 ){ // we are the child
    int status;
    if( seteuid(euid) == -1 )
      status = ERR_DISPATCH_F_SETEUID;
    else if( setegid(egid) == -1 )
      status = ERR_DISPATCH_F_SETEGID;
    else{
      status = (*f)(f_arg);
      if( status <= ERR_DISPATCH ) status = ERR_DISPATCH_NEGATIVE_RETURN_STATUS;
      exit(status);
    }
  }else{ // we are the parent
    uint err;
    waitpid(pid, &err, 0);
    return err;
  }
}

//--------------------------------------------------------------------------------
// interface call point, dispatch a executable
int dispatch_exec(char **argv, char **envp){
  char *command;
  {
    if( !argv || !argv[0] ) return ERR_DISPATCH_NULL_EXECUTABLE;
    #ifdef DEBUG
      dbprintf("dispatch_exec:");
      char **apt = argv;
      while( *apt ){
        dbprintf(" %s",*apt);
        apt++;
      }
      dbprintf("\n");
    #endif
    command = argv[0];
  }
  pid_t pid = fork();
  if( pid == -1 ) return ERR_DISPATCH_F_FORK; // something went wrong and we are still in the parent process
  if( pid == 0 ){ // we are the child
    execvpe(command, argv, envp); // exec will only return if it has an error
    #ifdef DEBUG
    dbprintf("dispatch_exec: exec returned, perror message:");
    perror("dispatch_exec"); // our only chance to print this message, as this is the child process
    #endif
    fflush(stdout);
    exit(ERR_DISPATCH_EXEC); 
  }else{ // we are the parent
    int err;
    waitpid(pid, &err, 0);
    return err;
  }
}

