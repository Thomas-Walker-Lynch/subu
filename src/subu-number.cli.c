/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subu-number.cli.h"
#include <stdio.h>


int print_subu_number(sqlite3 *db){
  uint msn;
  int ret = subu_number_get(db, &msn);
  if( ret == 0 ) printf("%u\n", msn);
  return ret;
}

int main(){
  int ret;
  sqlite3 *db;
  ret = sqlite3_open_v2(config_file, &db, SQLITE_OPEN_READWRITE, NULL);
  if( ret != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open configuration file\n");
    return ERR_CONFIG_FILE;
  }
  if( print_subu_number(db) == -1 ){
    fprintf(stderr, "error exit, could not read access maximum subunumber\n");
    return ERR_CONFIG_FILE;
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, strange, we could not close the configuration file\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
