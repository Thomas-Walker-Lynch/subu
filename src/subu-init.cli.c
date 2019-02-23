/*
This command initializes the configuration file.

*/
#include "subu-init.cli.h"
#include <stdio.h>

int main(){
  sqlite3 *db;
  if(
     sqlite3_open(config_file, &db)
     ||
     schema(db, 10)
  ){
    fprintf(stderr, "error exit, could not build schema\n");
    return ERR_CONFIG_FILE;
  }
  if( sqlite3_close(db) ){
    fprintf(stderr, "error exit, strange, we could not close the db\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
