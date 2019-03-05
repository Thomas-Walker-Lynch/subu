/*
  Forks a new process and runs the a function in that new process.  I.e. it 'spawns' a function.

  f must have the specified prototype. I.e. it accepts one void pointer and
  returns a positive integer. (Negative integers are used for dispatch error codes.)

  dispatch_f_euid_egid changes to the new euid and egid before running the function.

  If the change to in euid/egid fails, the forked process exits with a negative status.
  If the function has an error, it returns a positive status.  A status of zero means
  that all went well.

  Because f is running in a separate process, the return status is the only means
  of communication going back to the calling process.


*/
#define _GNU_SOURCE

#include "dispatch_f.lib.h"
// we need the declaration for uid_t etc.
// without this #define execvpe is undefined
#define _GNU_SOURCE   

#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#if INTERFACE
#include <sys/types.h>
#include <unistd.h>
#endif

//--------------------------------------------------------------------------------
// dispatch_f_ctx class
//
#if INTERFACE
#define ERR_NEGATIVE_RETURN_STATUS -1
#define ERR_DISPATCH_F_FORK -2
#define ERR_DISPATCH_F_SETEUID -3
#define ERR_DISPATCH_F_SETEGID -4

// both name and fname are static allocations
struct dispatch_f_ctx{
  char *dispatcher; // name of the dispatch function (currently "dispatch_f" or "dispatch_f_euid_egid")
  char *dispatchee; // name of the function being dispatched
  int err;
  int status; // return value from the function
};
#endif
dispatch_f_ctx *dispatch_f_ctx_mk(char *name, char *fname){
  dispatch_f_ctx *ctxp = malloc(sizeof(dispatch_f_ctx));
  ctxp->dispatcher = name;
  ctxp->dispatchee = fname;
  ctxp->err = 0;
  return ctxp;
}
void dispatch_f_ctx_free(dispatch_f_ctx *ctxp){
   // no dynamic variables to be freed in ctx
  free(ctxp); 
}
void dispatch_f_mess(struct dispatch_f_ctx *ctxp){
  if(ctxp->err == 0) return;
  switch(ctxp->err){
  case ERR_NEGATIVE_RETURN_STATUS:
    fprintf(stderr, "%s, function \"%s\" broke contract with a negative return value.", ctxp->dispatcher, ctxp->dispatchee);
    break;
  case ERR_DISPATCH_F_FORK:
  case ERR_DISPATCH_F_SETEUID:
  case ERR_DISPATCH_F_SETEGID:
    fprintf(stderr, "%s, ", ctxp->dispatcher);
    perror(ctxp->dispatcher);
    break;
  }
  fputc('\n', stderr);
}

//--------------------------------------------------------------------------------
// interface call point
dispatch_f_ctx *dispatch_f(char *fname, int (*f)(void *arg), void *f_arg){
  dispatch_f_ctx *ctxp = dispatch_f_ctx_mk("dispatch_f", fname);
  #ifdef DEBUG
  dbprintf("%s %s\n", ctxp->dispatcher, ctxp->dispatchee);
  #endif
  pid_t pid = fork();
  if( pid == -1 ){
    ctxp->err = ERR_DISPATCH_F_FORK; // we are still in the parent process
    return ctxp;
  }
  if( pid == 0 ){ // we are the child
    int status = (*f)(f_arg); // we require that f return a zero or positive value
    if( status < 0 ) status = ERR_NEGATIVE_RETURN_STATUS;
    exit(status);
  }else{ // we are the parent
    waitpid(pid, &(ctxp->status), 0);
    return ctxp;
  }
}

//--------------------------------------------------------------------------------
// interface call point
dispatch_f_ctx *dispatch_f_euid_egid(char *fname, int (*f)(void *arg), void *f_arg, uid_t euid, gid_t egid){
  dispatch_f_ctx *ctxp = dispatch_f_ctx_mk("dispatch_f_euid_egid", fname);
  #ifdef DEBUG
  dbprintf("%s %s as euid:%u egid:%u\n", ctxp->dispatcher, ctxp->dispatchee, euid, egid);
  #endif
  pid_t pid = fork();
  if( pid == -1 ){
    ctxp->err = ERR_DISPATCH_F_FORK;
    return ctxp;
  }
  if( pid == 0 ){ // we are the child
    int status;
    if( seteuid(euid) == -1 )
      status = ERR_DISPATCH_F_SETEUID;
    else if( setegid(egid) == -1 )
      status = ERR_DISPATCH_F_SETEGID;
    else{
      status = (*f)(f_arg);
      if( status < 0 ) status = ERR_NEGATIVE_RETURN_STATUS;
      exit(status);
    }
  }else{ // we are the parent
    waitpid(pid, &(ctxp->status), 0);
    return ctxp;
  }
}


