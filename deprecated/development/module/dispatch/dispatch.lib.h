/*
  Runs a command or function as its own process.

  The return status integer from command or function must be greater than ERR_DISPATCH.
  In the case of dispatch_exec, we only have a promise from the user to not dispatch
  non compliant commands.

*/
#ifndef DISPATCH_LIB_H
#define DISPATCH_LIB_H
#include <sys/types.h>
#include <unistd.h>

#define ERR_DISPATCH -1024
#define ERR_DISPATCH_NEGATIVE_RETURN_STATUS -1024
#define ERR_DISPATCH_F_FORK -1025
#define ERR_DISPATCH_F_SETEUID -1026
#define ERR_DISPATCH_F_SETEGID -1027
#define ERR_DISPATCH_NULL_EXECUTABLE -1028
#define ERR_DISPATCH_EXEC -1029

// currently both dispatcher and dispatchee strings are statically allocated
struct dispatch_ctx{
  char *dispatcher; // name of the dispatch function ("dispatch_f", "dispatch_f_euid_egid", etc.)
  char *dispatchee; // name of the function being dispatched
  int err; // error code as listed below, or status returned from dispatchee
};
void dispatch_f_mess(char *fname, int err, char *dispatchee);
int dispatch_f(char *fname, int (*f)(void *arg), void *f_arg);
int dispatch_f_euid_egid(char *fname, int (*f)(void *arg), void *f_arg, uid_t euid, gid_t egid);
int dispatch_exec(char **argv, char **envp);
