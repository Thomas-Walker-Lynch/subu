/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int subudb_Masteru_Subu_rm(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
typedef struct subudb_subu_element subudb_subu_element;
int subudb_Masteru_Subu_get_subus(sqlite3 *db,char *masteru_name,subudb_subu_element **sa_pt,subudb_subu_element **sa_end_pt);
void subu_element_free(subudb_subu_element *base,subudb_subu_element *end_pt);
struct subudb_subu_element {
  char *subuname;
  char *subu_username;
};
int subudb_Masteru_Subu_get_subu_username(sqlite3 *db,char *masteru_name,char *subuname,char **subu_username);
int subudb_Masteru_Subu_put(sqlite3 *db,char *masteru_name,char *subuname,char *subu_username);
int subudb_number_set(sqlite3 *db,int n);
int subudb_number_get(sqlite3 *db,int *n);
typedef unsigned int uint;
extern uint First_Max_Subunumber;
int subudb_schema(sqlite3 *db);
int db_rollback(sqlite3 *db);
int db_commit(sqlite3 *db);
int db_begin(sqlite3 *db);
#define INTERFACE 0
