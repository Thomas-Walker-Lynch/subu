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
#include <unistd.h>
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
#include <sqlite3.h>
extern char config_file[];
extern char config_file[];
int subu_mk_0(char *subuname,char *config_file);
int masteru_makes_subuhome(void *arg);
int allowed_subuname(char *subuname);
#define ERR_SUBU_MK_0_SETFACL 10
#define ERR_SUBU_MK_0_FAILED_USERADD 9
#define ERR_SUBU_MK_0_BUG_SSS 8
#define ERR_SUBU_MK_0_FAILED_MKDIR_SUBU 7
#define ERR_SUBU_MK_0_MK_SUBUHOME 6
#define ERR_SUBU_MK_0_MALLOC 5
#define ERR_SUBU_MK_0_BAD_MASTERU_HOME 4
#define ERR_SUBU_MK_0_SETUID_ROOT 3
#define ERR_SUBU_MK_0_CONFIG_FILE 2
#define ERR_SUBU_MK_0_ARG_CNT 1
#define INTERFACE 0
