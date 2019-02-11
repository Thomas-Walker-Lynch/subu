#ifndef USERADD_FI_H
#define USERADD_FI_H

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

// some value larger than any wstatus value
#define USERADD_ERR_UNDEFINED (1 << 16);
#define USERADD_ERR_ARGCNT (2 << 16);
#define USERADD_ERR_PWREC (3 << 16);

// only use pw_record if error is zero
struct useradd_ret{
  useradd_ret(error=USERADD_ERR_UNDEFINED,pwd=NULL):error(error),pwd(pwd){}
  int error;
  struct password *pw_record;// as per getpwnam man page do not free() this.
};
struct useradd_ret useradd(char **argv, char **envp);

#endif


