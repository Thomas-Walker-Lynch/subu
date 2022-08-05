#tranche subudb-init.cli.c
/*
This command initializes the db file.

*/
#include <stdio.h>
#include "common.lib.h"
#include "subudb.lib.h"
#include "subu.lib.h"

int main(){
  sqlite3 *db;
  if( sqlite3_open(DB_File, &db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open db file \"%s\"\n", DB_File);
    return SUBU_ERR_DB_FILE;
  }
  db_begin(db);
  if( subudb_schema(db) != SQLITE_OK ){
    db_rollback(db);
    fprintf(stderr, "error exit, opened db file but could not build schema\n");
    return SUBU_ERR_DB_FILE;
  }
  db_commit(db);
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not close the db\n");
    return SUBU_ERR_DB_FILE;
  }    
  return 0;
}
#tranche-end
