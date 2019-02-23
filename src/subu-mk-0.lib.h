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
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
#include <sqlite3.h>
extern char config_file[];
int subu_mk_0(char *subuname,char *config_file);
int masteru_makes_subuhome(void *arg);
int allowed_subuname(char *subuname);
#define ERR_SUBU_MK_0_SETFACL 9
#define ERR_SUBU_MK_0_FAILED_USERADD 8
#define ERR_SUBU_MK_0_BUG_SSS 7
#define ERR_SUBU_MK_0_FAILED_MKDIR_SUBU 6
#define ERR_SUBU_MK_0_MK_SUBUHOME 5
#define ERR_SUBU_MK_0_MALLOC 4
#define ERR_SUBU_MK_0_BAD_MASTERU_HOME 3
#define ERR_SUBU_MK_0_SETUID_ROOT 2
#define ERR_SUBU_MK_0_CONFIG_FILE 1
#define INTERFACE 0
