/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
int dispatch_exec(char **argv,char **envp);
struct dispatch_useradd_ret_t dispatch_useradd(char **argv,char **envp);
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
#include <sqlite3.h>
#define ERR_CONFIG_FILE -1
int dbprintf(const char *format,...);
extern char config_file[];
int subu_mk_0(char *subuname,char *config_file);
int masteru_makes_subuhome(void *arg);
int allowed_subuname(char *subuname);
typedef unsigned int uint;
