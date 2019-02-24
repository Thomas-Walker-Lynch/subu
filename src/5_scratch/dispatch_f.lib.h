/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sys/types.h>
#include <unistd.h>
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
int dispatch_f(char *fname,int(*f)(void *arg),void *f_arg);
#define ERR_DISPATCH_F_SETEGID 3
#define ERR_DISPATCH_F_SETEUID 2
#define ERR_DISPATCH_F_FORK 1
#define INTERFACE 0
