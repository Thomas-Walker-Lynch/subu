/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
int dispatch_exec(char **argv,char **envp);
#include <sys/types.h>
#include <pwd.h>
typedef struct dispatch_useradd_ret_t dispatch_useradd_ret_t;
typedef unsigned int uint;
struct dispatch_useradd_ret_t {
  uint error;
  struct passwd *pw_record;  
};
struct dispatch_useradd_ret_t dispatch_useradd(char **argv,char **envp);
#define ERR_DISPATCH_USERADD_PWREC 3
#define ERR_DISPATCH_USERADD_DISPATCH 2
#define ERR_DISPATCH_USERADD_ARGC 1
#define INTERFACE 0
