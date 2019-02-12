#ifndef DISPATCH_USERADD_LIB_H
#define DISPATCH_USERADD_LIB_H

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

// some value larger than any wstatus value
#define DISPATCH_USERADD_ERR_UNDEFINED 1;
#define DISPATCH_USERADD_ERR_ARGC 2;
#define DISPATCH_USERADD_ERR_PWREC 3;
#define DISPATCH_USERADD_ERR_DISPATCH 4;

// only use pw_record if error is zero
struct dispatch_useradd_ret_t{
  int error;
  struct passwd *pw_record;// as per getpwnam man page do not free() this.
};

struct dispatch_useradd_ret_t dispatch_useradd(char **argv, char **envp);

#endif


