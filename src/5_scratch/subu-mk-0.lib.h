/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int subu_put_masteru_subu(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
#include <sys/types.h>
#include <unistd.h>
typedef struct dispatch_ctx dispatch_ctx;
dispatch_ctx *dispatch_exec(char **argv,char **envp);
#define BUG_SSS_CACHE_RUID 1
void dispatch_f_mess(struct dispatch_ctx *ctxp);
#define ERR_DISPATCH -1024
dispatch_ctx *dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
struct dispatch_ctx {
  char *dispatcher; // name of the dispatch function ("dispatch_f", "dispatch_f_euid_egid", etc.)
  char *dispatchee; // name of the function being dispatched
  int err; // error code as listed below, or status returned from dispatchee
};
int subu_number_get(sqlite3 *db,char **nsp,char **errmsg);
extern char subuland_extension[];
int dbprintf(const char *format,...);
#include <stdbool.h>
#include <errno.h>
typedef struct subu_mk_0_ctx subu_mk_0_ctx;
struct subu_mk_0_ctx *subu_mk_0(sqlite3 *db,char *subuname);
typedef unsigned int uint;
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
#define ERR_SUBU_MK_0_FAILED_USERADD 10
#define ERR_SUBU_MK_0_BUG_SSS 9
#define ERR_SUBU_MK_0_SUBUHOME_EXISTS 8
#define ERR_SUBU_MK_0_CONFIG_FILE 7
#define ERR_SUBU_MK_0_MALLOC 6
#define ERR_SUBU_MK_0_MASTERU_HOMELESS 5
#define ERR_SUBU_MK_0_SETUID_ROOT 4
#define ERR_SUBU_MK_0_SUBUNAME_MALFORMED 3
#define ERR_SUBU_MK_0_RMDIR_SUBUHOME 2
#define ERR_SUBU_MK_0_MKDIR_SUBUHOME 1
#define INTERFACE 0
