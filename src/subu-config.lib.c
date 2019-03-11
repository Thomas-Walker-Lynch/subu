/*
The config file is maintained in SQLite

Because user names of are of limited length, subu user names are always named _s<number>.
A separate table translates the numbers into the subu names.

The first argument is the biggest subu number in the system, or one minus an 
starting point for subu numbering.

currently a unit converted to base 10 will always fit in a 21 bit buffer.

*/
#include "subu-config.lib.h"

#if INTERFACE
#include <sqlite3.h>
#define ERR_CONFIG_FILE 1
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------
int schema(sqlite3 *db, uint max_subu_number){
  char max_subu_number_string[32];
  uint max_subu_number_string_len = snprintf(max_subu_number_string, 32, "%u", max_subu_number);
  if( max_subu_number_string_len >= 32 ){
    fprintf(stderr, "error exit, max_subu_number too big to deal with\n");
    return ERR_CONFIG_FILE;
  }
  char sql1[] = "CREATE TABLE Master_Subu(masteru_name TEXT, subuname TEXT, subu_username TEXT); ";
  char sql2[] = "CREATE TABLE Key_Int(key TEXT, value INT); ";

  char sql3_1[] = "INSERT INTO Key_Int VALUES( 'max_subu_number', ";
  char sql3_2[] = " ); ";
  char sql3_len = strlen(sql3_1) + max_subu_number_string_len + strlen(sql3_2) + 1;
  char sql3[sql3_len];
  strcpy(sql3, sql3_1);
  strcpy(sql3 + strlen(sql3_1), max_subu_number_string);
  strcpy(sql3 + strlen(sql3_1) + max_subu_number_string_len, sql3_2);

  char sql[strlen(sql1) + strlen(sql2) + strlen(sql3) + 1];
  strcpy(sql, sql1);
  strcpy(sql + strlen(sql1), sql2);
  strcpy(sql + strlen(sql1) + strlen(sql2), sql3);

  return sqlite3_exec(db, sql, NULL, NULL, NULL);
}

//--------------------------------------------------------------------------------

// the call back for subu_number_next, note also 3_doc/sqlite3.txt 
static int subu_number_extract(void *nsp, int colcnt, char **colvals, char **colnames){
  if(colcnt >= 1){
    *(char **)nsp = strdup( colvals[0] );
    return 0;
  }
  return -1;
}
int subu_number_next(sqlite3 *db, char **nsp, char **mess){
  char *sql = 
    "BEGIN TRANSACTION;"
    "UPDATE Key_Int SET value = value + 1 WHERE key = 'max_subu_number';"
    "SELECT value FROM Key_Int WHERE key = 'max_subu_number';"
    "COMMIT;";
  int rc = sqlite3_exec(db, sql, subu_number_extract, (void *)nsp, mess);
  return rc;
}
int subu_number_get(sqlite3 *db, int *n){
  char *sql = "SELECT value FROM Key_Int WHERE key = 'max_subu_number';";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  int rc = sqlite3_step(stmt);
  if( rc == SQLITE_ROW ){
    *n = sqlite3_column_int(stmt,0);
  }else{
    sqlite3_finalize(stmt);
    return rc; // woops this needs to return an error!,  be sure it is not SQLITE_DONE
  }
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc;
}
int subu_number_set(sqlite3 *db, int n){
  char *sql = "UPDATE Key_Int SET value = ?1 WHERE key = 'max_subu_number';";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_int(stmt, 1, n);
  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc;
}


//--------------------------------------------------------------------------------
int subu_put_masteru_subu(sqlite3 *db, char *masteru_name, char *subuname, char *subu_username){
  char *sql = "INSERT INTO Master_Subu VALUES (?1, ?2, ?3);";
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
int subu_get_masteru_subu(sqlite3 *db, char *masteru_name, char *subuname, char **subu_username){
  char *sql = "SELECT subu_username FROM Master_Subu WHERE masteru_name = ?1 AND subuname = ?2;";
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
int subu_rm_masteru_subu(sqlite3 *db, char *masteru_name, char *subuname, char *subu_username){
  char *sql = "DELETE FROM Master_Subu WHERE masteru_name = ?1 AND subuname = ?2 AND subu_username = ?3;";
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
