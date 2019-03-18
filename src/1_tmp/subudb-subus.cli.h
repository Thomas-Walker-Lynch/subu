/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
typedef struct subudb_subu_element subudb_subu_element;
int subudb_Masteru_Subu_get_subus(sqlite3 *db,char *masteru_name,subudb_subu_element **sa_pt,subudb_subu_element **sa_end_pt);
struct subudb_subu_element {
  char *subuname;
  char *subu_username;
};
#include <stdbool.h>
#include <errno.h>
#define SUBU_ERR_DB_FILE 8
extern char DB_File[];
#define SUBU_ERR_ARG_CNT 1
