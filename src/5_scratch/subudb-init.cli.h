/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <sqlite3.h>
int db_commit(sqlite3 *db);
int db_rollback(sqlite3 *db);
int subudb_schema(sqlite3 *db);
int db_begin(sqlite3 *db);
#include <stdbool.h>
#include <errno.h>
#define SUBU_ERR_DB_FILE 8
extern char DB_File[];
