/*
  Runs a command or function as its own process.

  For the return status from command or function must be greater than ERR_DISPATCH.
  In the case of dispatch_exec, we only have a promise from the user to not dispatch
  non compliant commands.

  Interesting it isn't possible to have a kernel dispatch that calls fork, and then
  others that call this kernel.  The reason being is because of collision in return
  status values between the function in the kernel, and with each wrapper.  The 
  TTCA with the passed in error processing functions would not have this problem.

*/
#define _GNU_SOURCE

#include "dispatch.lib.h"

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
// dispatch returns an instance of this class.  
//
#if INTERFACE
// currently both dispatcher and dispatchee strings are statically allocated
struct dispatch_ctx{
  char *dispatcher; // name of the dispatch function ("dispatch_f", "dispatch_f_euid_egid", etc.)
  char *dispatchee; // name of the function being dispatched
  int err; // error code as listed below, or status returned from dispatchee
};
#define ERR_DISPATCH -1024
#define ERR_DISPATCH_NEGATIVE_RETURN_STATUS -1024
#define ERR_DISPATCH_F_FORK -1025
#define ERR_DISPATCH_F_SETEUID -1026
#define ERR_DISPATCH_F_SETEGID -1027
#define ERR_DISPATCH_NULL_EXECUTABLE -1028
#define ERR_DISPATCH_EXEC -1029
#endif
dispatch_ctx *dispatch_ctx_mk(char *dispatcher, char *dispatchee){
  dispatch_ctx *ctxp = malloc(sizeof(dispatch_ctx));
  ctxp->dispatcher = dispatcher;
  ctxp->dispatchee = dispatchee;
  ctxp->err = 0;
  return ctxp;
}
void dispatch_ctx_free(dispatch_ctx *ctxp){
   // currently no dynamic variables to be freed in ctx
  free(ctxp); 
}
void dispatch_f_mess(struct dispatch_ctx *ctxp){
  if(ctxp->err > ERR_DISPATCH) return;
  fprintf(stderr, "%s", ctxp->dispatcher); // if fprintf gets an error, errno will be overwritten
  switch(ctxp->err){
  case ERR_DISPATCH_NEGATIVE_RETURN_STATUS:
    fprintf(stderr, " dispatchee \"%s\" returned a negative status.", ctxp->dispatchee);
    break;
  case ERR_DISPATCH_F_FORK:
  case ERR_DISPATCH_F_SETEUID:
  case ERR_DISPATCH_F_SETEGID:
    fputc(' ', stderr);
    perror(ctxp->dispatcher);
    break;
  case ERR_DISPATCH_NULL_EXECUTABLE:
    fprintf(stderr, " executable was not specified");
    break;
  case ERR_DISPATCH_EXEC:
    // exec is in another process when it fails, so we can't see the errno value it set
    fprintf(stderr, " exec of \"%s\" failed", ctxp->dispatchee);
    break;
  default:
    fprintf(stderr, " returned undefined status");
  }
  fputc('\n', stderr);
}

//--------------------------------------------------------------------------------
// interface call point, dispatch a function
dispatch_ctx *dispatch_f(char *fname, int (*f)(void *arg), void *f_arg){
  dispatch_ctx *ctxp = dispatch_ctx_mk("dispatch_f", fname);
  #ifdef DEBUG
  dbprintf("%s %s\n", ctxp->dispatcher, ctxp->dispatchee);
  #endif
  pid_t pid = fork();
  if( pid == -1 ){
    ctxp->err = ERR_DISPATCH_F_FORK; // something went wrong and we are still in the parent process
    return ctxp;
  }
  if( pid == 0 ){ // we are the child
    int status = (*f)(f_arg); // we require that f return a zero or positive value
    if( status < ERR_DISPATCH ) status = ERR_DISPATCH_NEGATIVE_RETURN_STATUS;
    exit(status);
  }else{ // we are the parent
    waitpid(pid, &(ctxp->err), 0);
    return ctxp;
  }
}

//--------------------------------------------------------------------------------
// interface call point, dispatch a function with a given euid/egid
// of course this will only work if our euid is root in the first place
dispatch_ctx *dispatch_f_euid_egid(char *fname, int (*f)(void *arg), void *f_arg, uid_t euid, gid_t egid){
  dispatch_ctx *ctxp = dispatch_ctx_mk("dispatch_f_euid_egid", fname);
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
      if( status <= ERR_DISPATCH ) status = ERR_DISPATCH_NEGATIVE_RETURN_STATUS;
      exit(status);
    }
  }else{ // we are the parent
    waitpid(pid, &(ctxp->err), 0);
    return ctxp;
  }
}

//--------------------------------------------------------------------------------
// interface call point, dispatch a executable
dispatch_ctx *dispatch_exec(char **argv, char **envp){
  dispatch_ctx *ctxp;
  char *command;
  {
    if( !argv || !argv[0] ){
      ctxp = dispatch_ctx_mk("dispatch_exec", "NULL");
      ctxp->err = ERR_DISPATCH_NULL_EXECUTABLE;
      return ctxp;
    }
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
    ctxp = dispatch_ctx_mk("dispatch_exec", command);
  }
  pid_t pid = fork();
  if( pid == -1 ){
    ctxp->err = ERR_DISPATCH_F_FORK; // something went wrong and we are still in the parent process
    return ctxp;
  }
  if( pid == 0 ){ // we are the child
    execvpe(command, argv, envp); // exec will only return if it has an error
    perror("dispatch_exec"); // our only chance to print this message, as this is the child process
    exit(ERR_DISPATCH_EXEC); 
  }else{ // we are the parent
    waitpid(pid, &(ctxp->err), 0);
    return ctxp;
  }
}


