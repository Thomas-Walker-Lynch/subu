/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int subudb_Masteru_Subu_rm(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
int subudb_Masteru_Subu_get(sqlite3 *db,char *masteru_name,char *subuname,char **subu_username);
int subudb_Masteru_Subu_put(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
int subudb_number_rm(sqlite3 *db,char *masteru_name);
int subudb_number_set(sqlite3 *db,char *masteru_name,int n);
int subudb_number_get(sqlite3 *db,char *masteru_name,int *n);
int subudb_number_init(sqlite3 *db,char *masteru_name,int n);
int subudb_schema(sqlite3 *db);
int db_rollback(sqlite3 *db);
int db_commit(sqlite3 *db);
int db_begin(sqlite3 *db);
#define INTERFACE 0
