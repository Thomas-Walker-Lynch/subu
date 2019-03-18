/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sys/types.h>
#include <unistd.h>
int dispatch_exec(char **argv,char **envp);
typedef unsigned int uint;
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
int dispatch_f(char *fname,int(*f)(void *arg),void *f_arg);
void dispatch_f_mess(char *fname,int err,char *dispatchee);
#define ERR_DISPATCH_EXEC -1029
#define ERR_DISPATCH_NULL_EXECUTABLE -1028
#define ERR_DISPATCH_F_SETEGID -1027
#define ERR_DISPATCH_F_SETEUID -1026
#define ERR_DISPATCH_F_FORK -1025
#define ERR_DISPATCH_NEGATIVE_RETURN_STATUS -1024
#define ERR_DISPATCH -1024
typedef struct dispatch_ctx dispatch_ctx;
struct dispatch_ctx {
  char *dispatcher; // name of the dispatch function ("dispatch_f", "dispatch_f_euid_egid", etc.)
  char *dispatchee; // name of the function being dispatched
  int err; // error code as listed below, or status returned from dispatchee
};
#define INTERFACE 0
