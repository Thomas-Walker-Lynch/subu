/*
This command optionally sets the subu number in the config file, and prints its value.

Currently it doesn't do the setting part.

*/
#include "subu-number.cli.h"
#include <stdio.h>

int main(){
  int ret;
  sqlite3 *db;
  ret = sqlite3_open(config_file, &db);
  if( ret != SQLITE_OK ){
    fprintf(stderr, "error exit, could not build schema\n");
    return ERR_CONFIG_FILE;
  }
  uint subu_number;
  ret = subu_number(db, subu_number);
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, strange, we could not close the db\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
