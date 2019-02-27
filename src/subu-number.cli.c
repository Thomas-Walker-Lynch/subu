/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subu-number.cli.h"
#include <stdio.h>


int print_subu_number(sqlite3 *db){
  return ret;
}

int main(){
  sqlite3 *db;
  {
    int ret = sqlite3_open_v2(config_file, &db, SQLITE_OPEN_READWRITE, NULL);
    if( ret != SQLITE_OK ){
      fprintf(stderr, "error exit, could not open configuration file\n");
      return ERR_CONFIG_FILE;
    }}

  char *n;
  char *mess;
  int ret = subu_number_get(db, &n, &mess);
  if( mess ){
    fprintf(stderr, "subu_number, %s\n", mess);
    free(mess);
  }
  if( ret == SQLITE_OK ){
    printf("%s\n", n);
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, strange, we could not close the configuration file\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
