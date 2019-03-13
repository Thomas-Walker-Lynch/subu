/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <stdbool.h>
#include <errno.h>
#include <sqlite3.h>
int subu_bind(char **mess,char *masteru_name,char *subu_username,char *subuhome);
int subudb_Masteru_Subu_rm(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
int subudb_Masteru_Subu_get_subu_username(sqlite3 *db,char *masteru_name,char *subuname,char **subu_username);
int subu_rm_0(char **mess,sqlite3 *db,char *subuname);
int subudb_Masteru_Subu_put(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
#include <sys/types.h>
#include <unistd.h>
int dispatch_exec(char **argv,char **envp);
#define BUG_SSS_CACHE_RUID 1
void dispatch_f_mess(char *fname,int err,char *dispatchee);
#define ERR_DISPATCH -1024
int dispatch_f_euid_egid(char *fname,int(*f)(void *arg),void *f_arg,uid_t euid,gid_t egid);
int dbprintf(const char *format,...);
int subu_mk_0(char **mess,sqlite3 *db,char *subuname);
extern char Subuland_Extension[];
int db_commit(sqlite3 *db);
int db_rollback(sqlite3 *db);
int subudb_number_set(sqlite3 *db,int n);
int subudb_number_get(sqlite3 *db,int *n);
int db_begin(sqlite3 *db);
typedef unsigned int uint;
extern uint Subuhome_Perms;
extern char DB_File[];
void subu_err(char *fname,int err,char *mess);
#define SUBU_ERR_BIND 15
#define SUBU_ERR_N 14
#define SUBU_ERR_SUBU_NOT_FOUND 13
#define SUBU_ERR_FAILED_USERDEL 12
#define SUBU_ERR_FAILED_USERADD 11
#define SUBU_ERR_BUG_SSS 10
#define SUBU_ERR_SUBUHOME_EXISTS 9
#define SUBU_ERR_DB_FILE 8
#define SUBU_ERR_HOMELESS 7
#define SUBU_ERR_SUBUNAME_MALFORMED 6
#define SUBU_ERR_RMDIR_SUBUHOME 5
#define SUBU_ERR_MKDIR_SUBUHOME 4
#define SUBU_ERR_MALLOC 3
#define SUBU_ERR_SETUID_ROOT 2
#define SUBU_ERR_ARG_CNT 1
char *userdel_mess(int err);
char *useradd_mess(int err);
#define INTERFACE 0
