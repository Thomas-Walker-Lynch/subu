/*
This command optionally sets the subu number in the config file, and prints its value.

Currently it doesn't do the setting part.

*/
#include "subu-number.cli.h"
#include <stdio.h>

int main(int argc, char **argv, char **envp){
  int ret;
  sqlite3 *db;
  ret = sqlite3_open(config_file, &db);
  if( ret != SQLITE_OK ){
    fprintf(stderr, "error exit, could not build schema\n");
    return ERR_CONFIG_FILE;
  }
  uint msn;
  ret = subu_number(db, &msn);
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, strange, we could not close the db\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
