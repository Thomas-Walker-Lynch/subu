/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sys/types.h>
#include <unistd.h>
typedef struct dispatch_f_ctx dispatch_f_ctx;
dispatch_f_ctx *dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
dispatch_f_ctx *dispatch_f(char *fname,int(*f)(void *arg),void *f_arg);
void dispatch_f_mess(struct dispatch_f_ctx *ctxp);
void dispatch_f_ctx_free(dispatch_f_ctx *ctxp);
dispatch_f_ctx *dispatch_f_ctx_mk(char *name,char *fname);
struct dispatch_f_ctx {
  char *dispatcher; // name of the dispatch function (currently "dispatch_f" or "dispatch_f_euid_egid")
  char *dispatchee; // name of the function being dispatched
  int err;
  int status; // return value from the function
};
#define ERR_DISPATCH_F_SETEGID -4
#define ERR_DISPATCH_F_SETEUID -3
#define ERR_DISPATCH_F_FORK -2
#define ERR_NEGATIVE_RETURN_STATUS -1
#define INTERFACE 0
