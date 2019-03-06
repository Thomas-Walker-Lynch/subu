/*
This command initializes the configuration file.

*/
#include "subu-init.cli.h"
#include <stdio.h>

int main(){
  sqlite3 *db;
  if( sqlite3_open_v2(config_file, &db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open configuration file\n");
    return ERR_CONFIG_FILE;
  }
  if( schema(db, first_max_subu_number) != SQLITE_OK ){
    fprintf(stderr, "error exit, opened config file but could not build scheme\n");
    return ERR_CONFIG_FILE;
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, could not close the db\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
