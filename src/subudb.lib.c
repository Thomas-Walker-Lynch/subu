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
#include <stdbool.h>

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
  int rc;

  { // build tables
    char sql[] = 
      "CREATE TABLE Masteru_Subu(masteru_name TEXT, subuname TEXT, subu_username TEXT);"
      "CREATE TABLE Attribute_Int(attribute TEXT, value INT);"
      ;
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if(rc != SQLITE_OK) return rc;
  }

  { // data initialization
    char *sql = "INSERT INTO Attribute_Int (attribute, value) VALUES ('Max_Subunumber', ?1);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, First_Max_Subunumber);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if( rc != SQLITE_DONE ) return rc;
  }

  return SQLITE_OK;
}

//--------------------------------------------------------------------------------
int subudb_number_get(sqlite3 *db, int *n){
  char *sql = "SELECT value FROM Attribute_Int WHERE attribute = 'Max_Subunumber';";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  int rc = sqlite3_step(stmt);
  if( rc == SQLITE_ROW ){
    *n = sqlite3_column_int(stmt,0);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if( rc != SQLITE_DONE ) return rc;
    return SQLITE_OK;
  }
  // should have a message return, suppose
  sqlite3_finalize(stmt);
  return SQLITE_NOTFOUND; 
}

int subudb_number_set(sqlite3 *db, int n){
  int rc;
  char *sql = "UPDATE Attribute_Int SET value = ?1 WHERE attribute = 'Max_Subunumber';";
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  sqlite3_bind_int(stmt, 1, n);
  rc = sqlite3_step(stmt);
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
  if( rc == SQLITE_DONE ) return SQLITE_OK;
  return rc;
}

//--------------------------------------------------------------------------------
int subudb_Masteru_Subu_get_subu_username(sqlite3 *db, char *masteru_name, char *subuname, char **subu_username){
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
  if( rc == SQLITE_DONE ) return SQLITE_OK;
  return rc;
}

//--------------------------------------------------------------------------------

// we return and array of subudb_subu_info
#if INTERFACE
struct subudb_subu_element{
  char *subuname;
  char *subu_username;
};
#endif
static void subu_element_alloc(subudb_subu_element **base, size_t *s){
  dalloc((void *)base, s, sizeof(subudb_subu_element));
}
void subu_element_free(subudb_subu_element *base, subudb_subu_element *end_pt){
  subudb_subu_element *pt = base;
  while( pt != end_pt ){
    free(pt->subuname);
    free(pt->subu_username);
  pt++;
  }
  free(base);  
}

int subudb_Masteru_Subu_get_subus
(
 sqlite3 *db,
 char *masteru_name,
 subudb_subu_element **sa_pt,
 subudb_subu_element **sa_end_pt
){
  char *sql = "SELECT subuname, subu_username"
              " FROM Masteru_Subu"
              " WHERE masteru_name = ?1;";
  size_t sql_len = strlen(sql);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db, sql, sql_len, &stmt, NULL); 
  if( rc != SQLITE_OK ) return rc;
  sqlite3_bind_text(stmt, 1, masteru_name, strlen(masteru_name), SQLITE_STATIC);

  size_t subu_element_size;
  subudb_subu_element *subu_element;
  subu_element_alloc(&subu_element, &subu_element_size);
  subudb_subu_element *pt = subu_element;
  rc = sqlite3_step(stmt);
  while( rc == SQLITE_ROW ){
    if( off_alloc(subu_element, pt, subu_element_size) ) expand((void **)&subu_element, (void **)&pt, &subu_element_size);
    pt->subuname = strdup(sqlite3_column_text(stmt, 0));
    pt->subu_username = strdup(sqlite3_column_text(stmt, 1));
  rc = sqlite3_step(stmt);
  pt++;
  }
  sqlite3_finalize(stmt);
  if( rc != SQLITE_DONE ){
    return rc; // woops this needs to return an error!,  be sure it is not SQLITE_DONE
  }
  *sa_pt = subu_element;
  *sa_end_pt = pt;
  return SQLITE_OK;
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
  if( rc == SQLITE_DONE ) return SQLITE_OK;
  return rc;
}
