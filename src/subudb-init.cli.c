/*
This command initializes the db file.

*/
#include "subudb-init.cli.h"
#include <stdio.h>

int main(){
  sqlite3 *db;
  if( sqlite3_open(DB_File, &db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open db file \"%s\"\n", DB_File);
    return SUBU_ERR_DB_FILE;
  }
  if( subudb_schema(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, opened db file but could not build schema\n");
    return SUBU_ERR_DB_FILE;
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not close the db\n");
    return SUBU_ERR_DB_FILE;
  }    
  return 0;
}
