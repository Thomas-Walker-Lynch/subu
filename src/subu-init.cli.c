/*
This command initializes the configuration file.

*/
#include "subu-init.cli.h"
#include <stdio.h>

int main(){
  sqlite3 *db;
  if( sqlite3_open(Config_File, &db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open configuration file \"%s\"\n", Config_File);
    return ERR_CONFIG_FILE;
  }
  if( subudb_schema(db, First_Max_Subu_number) != SQLITE_OK ){
    fprintf(stderr, "error exit, opened config file but could not build scheme\n");
    return ERR_CONFIG_FILE;
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not close the db\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
