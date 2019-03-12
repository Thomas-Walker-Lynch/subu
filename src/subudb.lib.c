/*
The db file is maintained in SQLite

Because user names of are of limited length, subu user names are always named _s<number>.
A separate table translates the numbers into the subu names.

The first argument is the biggest subu number in the system, or one minus an 
starting point for subu numbering.

currently a unit converted to base 10 will always fit in a 21 bit buffer.

Each of these returns SQLITE_OK upon success
*/
#include "subudb.lib.h"

#if INTERFACE
#include <sqlite3.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------
// sqlite transactions don't nest.  There is a way to use save points, but still
// we can't just nest transactions.  Instead use these wrappers around the whole
// of something that needs to be in a transaction.
int db_begin(sqlite3 *db){
  return sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}
int db_commit(sqlite3 *db){
  return sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
}
int db_rollback(sqlite3 *db){
  return sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
}

//--------------------------------------------------------------------------------
int subudb_schema(sqlite3 *db){
  char sql[] = 
    "CREATE TABLE Masteru_Subu(masteru_name TEXT, subuname TEXT, subu_username TEXT);"
    "CREATE TABLE Masteru_Max(masteru_name TEXT, max_subu_number INT);"
    ;
  return sqlite3_exec(db, sql, NULL, NULL, NULL);
}

//--------------------------------------------------------------------------------
int subudb_number_init(sqlite3 *db, char *masteru_name, int n){
  int rc;
  char *sql = "INSERT INTO Masteru_Max (masteru_name, max_subu_number) VALUES (?1, ?2);";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, masteru_name, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, n);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if( rc == SQLITE_DONE ) return SQLITE_OK;
  return rc;
}

int subudb_number_get(sqlite3 *db, char *masteru_name, int *n){
  char *sql = "SELECT max_subu_number FROM Masteru_Max WHERE masteru_name = ?1;";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, masteru_name, -1, SQLITE_STATIC);
  int rc = sqlite3_step(stmt);
  if( rc == SQLITE_ROW ){
    *n = sqlite3_column_int(stmt,0);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if( rc == SQLITE_DONE ) return SQLITE_OK;
    return rc;
  }
  // should have a message return, suppose
  sqlite3_finalize(stmt);
  return SQLITE_NOTFOUND; 
}

// on success returns SQLITE_DONE
int subudb_number_set(sqlite3 *db, char *masteru_name, int n){
  int rc;
  char *sql = "UPDATE Masteru_Max SET max_subu_number = ?1 WHERE masteru_name = ?2;";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_int(stmt, 1, n);
  sqlite3_bind_text(stmt, 2, masteru_name, -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if( rc == SQLITE_DONE ) return SQLITE_OK;
  return rc;
}

// returns SQLITE_DONE or an error code
// removes masteru/max_number relation from table
int subudb_number_rm(sqlite3 *db, char *masteru_name){
  char *sql = "DELETE FROM Masteru_Max WHERE masteru_name = ?1;";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, masteru_name, -1, SQLITE_STATIC);
  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if( rc == SQLITE_DONE ) return SQLITE_OK;
  return rc;
}

//--------------------------------------------------------------------------------
// put relation into Masteru_Subu table
int subudb_Masteru_Subu_put(sqlite3 *db, char *masteru_name, char *subuname, char *subu_username){
  char *sql = "INSERT INTO Masteru_Subu VALUES (?1, ?2, ?3);";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, masteru_name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, subuname, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, subu_username, -1, SQLITE_STATIC);
  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc;
}

//--------------------------------------------------------------------------------
int subudb_Masteru_Subu_get(sqlite3 *db, char *masteru_name, char *subuname, char **subu_username){
  char *sql = "SELECT subu_username FROM Masteru_Subu WHERE masteru_name = ?1 AND subuname = ?2;";
  size_t sql_len = strlen(sql);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db, sql, sql_len, &stmt, NULL); 
  if( rc != SQLITE_OK ) return rc;
  sqlite3_bind_text(stmt, 1, masteru_name, strlen(masteru_name), SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, subuname, strlen(subuname), SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  if( rc == SQLITE_ROW ){
    const char *username = sqlite3_column_text(stmt, 0);
    *subu_username = strdup(username);
  }else{
    sqlite3_finalize(stmt);
    return rc; // woops this needs to return an error!,  be sure it is not SQLITE_DONE
  }
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc;
}

//--------------------------------------------------------------------------------
int subudb_Masteru_Subu_rm(sqlite3 *db, char *masteru_name, char *subuname, char *subu_username){
  char *sql = "DELETE FROM Masteru_Subu WHERE masteru_name = ?1 AND subuname = ?2 AND subu_username = ?3;";
  size_t sql_len = strlen(sql);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db, sql, sql_len, &stmt, NULL); 
  if( rc != SQLITE_OK ) return rc;
  sqlite3_bind_text(stmt, 1, masteru_name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, subuname, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, subu_username, -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc;
}
