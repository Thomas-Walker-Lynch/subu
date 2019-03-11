/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int subu_rm_masteru_subu(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
int subu_get_masteru_subu(sqlite3 *db,char *masteru_name,char *subuname,char **subu_username);
int subu_put_masteru_subu(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
int subu_number_set(sqlite3 *db,int n);
int subu_number_get(sqlite3 *db,int *n);
int subu_number_next(sqlite3 *db,char **nsp,char **mess);
typedef unsigned int uint;
int schema(sqlite3 *db,uint max_subu_number);
#define ERR_CONFIG_FILE 1
#define INTERFACE 0
