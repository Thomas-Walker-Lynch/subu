/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int subu_put_masteru_subu(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
#include <sys/types.h>
#include <unistd.h>
int dispatch_exec(char **argv,char **envp);
#define BUG_SSS_CACHE_RUID 1
void dispatch_f_mess(char *fname,int err,char *dispatchee);
#define ERR_DISPATCH -1024
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
#include <stdbool.h>
#include <errno.h>
int subu_mk_0(char **mess,sqlite3 *db,char *subuname);
extern char Subuland_Extension[];
int subu_number_get(sqlite3 *db,char **nsp,char **errmsg);
typedef unsigned int uint;
extern uint Subuhome_Perms;
extern char Config_File[];
void subu_err(char *fname,int err,char *mess);
#define SUBU_ERR_FAILED_USERADD 11
#define SUBU_ERR_BUG_SSS 10
#define SUBU_ERR_SUBUHOME_EXISTS 9
#define SUBU_ERR_CONFIG_FILE 8
#define SUBU_ERR_MASTERU_HOMELESS 7
#define SUBU_ERR_SUBUNAME_MALFORMED 5
#define SUBU_ERR_RMDIR_SUBUHOME 4
#define SUBU_ERR_MKDIR_SUBUHOME 3
#define SUBU_ERR_MALLOC 1
#define SUBU_ERR_SETUID_ROOT 6
#define SUBU_ERR_ARG_CNT 2
char *useradd_mess(int err);
#define INTERFACE 0
