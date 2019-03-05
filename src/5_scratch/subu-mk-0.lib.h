/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int subu_put_masteru_subu(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
#include <sys/types.h>
#include <pwd.h>
typedef struct dispatch_useradd_ret_t dispatch_useradd_ret_t;
typedef unsigned int uint;
struct dispatch_useradd_ret_t {
  uint error;
  struct passwd *pw_record;  
};
struct dispatch_useradd_ret_t dispatch_useradd(char **argv,char **envp);
#define BUG_SSS_CACHE_RUID 1
#include <unistd.h>
typedef struct dispatch_f_ctx dispatch_f_ctx;
dispatch_f_ctx *dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
struct dispatch_f_ctx {
  char *dispatcher; // name of the dispatch function (currently "dispatch_f" or "dispatch_f_euid_egid")
  char *dispatchee; // name of the function being dispatched
  int err;
  int status; // return value from the function
};
int subu_number_get(sqlite3 *db,char **nsp,char **errmsg);
int dbprintf(const char *format,...);
#include <stdbool.h>
#include <errno.h>
typedef struct subu_mk_0_ctx subu_mk_0_ctx;
struct subu_mk_0_ctx *subu_mk_0(sqlite3 *db,char *subuname);
extern uint subuhome_perms;
void subu_mk_0_mess(struct subu_mk_0_ctx *ctxp);
void subu_mk_0_ctx_free(struct subu_mk_0_ctx *ctxp);
struct subu_mk_0_ctx *subu_mk_0_ctx_mk();
struct subu_mk_0_ctx {
  char *name;
  char *subuland;
  char *subuhome;
  char *subu_username;
  bool free_aux;
  char *aux;
  uint err;
};
#define ERR_SUBU_MK_0_FAILED_USERADD 9
#define ERR_SUBU_MK_0_BUG_SSS 8
#define ERR_SUBU_MK_0_SUBUHOME_EXISTS 7
#define ERR_SUBU_MK_0_CONFIG_FILE 6
#define ERR_SUBU_MK_0_MALLOC 5
#define ERR_SUBU_MK_0_MASTERU_HOMELESS 4
#define ERR_SUBU_MK_0_SETUID_ROOT 3
#define ERR_SUBU_MK_0_SUBUNAME_MALFORMED 2
#define ERR_SUBU_MK_0_MKDIR_SUBUHOME 1
#define INTERFACE 0
