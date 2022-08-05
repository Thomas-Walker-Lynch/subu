  typedef unsigned int uint;
  /*
    Fedora 29's sss_cache is checking the inherited uid instead of the effective
    uid, so setuid root scripts will fail when calling sss_cache.

    Fedora 29's 'useradd' calls sss_cache, and useradd is called by our setuid root 
    program subu-mk-0.
  */
  #define BUG_SSS_CACHE_RUID 1
  // extern char *DB_File;
  extern char DB_File[];
  extern uint Subuhome_Perms;
  extern uint First_Max_Subunumber;
  extern char Subuland_Extension[];
  /*
  The db file is maintained in SQLite

  Because linux user names are limited length, subu user names are of a compact
  form: _s<number>.  A separate table translates the numbers into the subu names.

  Each of these returns SQLITE_OK upon success
  */
  #include <da.h>
  #include <sqlite3.h>
  int db_begin(sqlite3 *db);
  int db_commit(sqlite3 *db);
  int db_rollback(sqlite3 *db);
  int subudb_schema(sqlite3 *db);
  int subudb_number_get(sqlite3 *db, int *n);
  int subudb_number_set(sqlite3 *db, int n);
  int subudb_Masteru_Subu_put(sqlite3 *db, char *masteru_name, char *subuname, char *subu_username);
  int subudb_Masteru_Subu_get_subu_username(sqlite3 *db, char *masteru_name, char *subuname, char **subu_username);
  typedef struct{
    char *subuname; // the name that masteru chose for his or her subu
    char *subu_username;  // the adduser name we gave it, typically of the s<number>
  } subudb_subu_element;
  int subudb_Masteru_Subu_get_subus(sqlite3 *db, char *masteru_name, Da *subus);
  int subudb_Masteru_Subu_rm(sqlite3 *db, char *masteru_name, char *subuname, char *subu_username);
  #include <stdbool.h>
  #include <errno.h>
  #include <sqlite3.h>
  char *useradd_mess(int err);
  char *userdel_mess(int err);
  #define SUBU_ERR_ARG_CNT 1
  #define SUBU_ERR_SETUID_ROOT 2
  #define SUBU_ERR_MALLOC 3
  #define SUBU_ERR_MKDIR_SUBUHOME 4
  #define SUBU_ERR_RMDIR_SUBUHOME 5
  #define SUBU_ERR_SUBUNAME_MALFORMED 6
  #define SUBU_ERR_HOMELESS 7
  #define SUBU_ERR_DB_FILE 8
  #define SUBU_ERR_SUBUHOME_EXISTS 9
  #define SUBU_ERR_BUG_SSS 10
  #define SUBU_ERR_FAILED_USERADD 11
  #define SUBU_ERR_FAILED_USERDEL 12
  #define SUBU_ERR_SUBU_NOT_FOUND 13
  #define SUBU_ERR_N 14
  #define SUBU_ERR_BIND 15
  void subu_err(char *fname, int err, char *mess);
  int subu_mk_0(char **mess, sqlite3 *db, char *subuname);
  int subu_rm_0(char **mess, sqlite3 *db, char *subuname);
  int subu_bind(char **mess, char *masteru_name, char *subu_username, char *subuhome);
  int subu_bind_all(char **mess, sqlite3 *db);
