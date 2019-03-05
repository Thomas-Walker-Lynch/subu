/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subu-put.cli.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){

  if(argc != 4){
    fprintf(stderr, "expected: %s masteru_name subuname subu_username\n", argv[0]);
    return 1;
  }
  char *masteru_name = argv[1];
  char *subuname = argv[2];
  char *subu_username = argv[3];

  sqlite3 *db;
  {
    int ret = sqlite3_open_v2(config_file, &db, SQLITE_OPEN_READWRITE, NULL);
    if( ret != SQLITE_OK ){
      fprintf(stderr, "error exit, could not open configuration file\n");
      return ERR_CONFIG_FILE;
    }}

  int ret = subu_put_masteru_subu(db, masteru_name, subuname, subu_username);
  if( ret != SQLITE_DONE ){
    printf("put failed\n");
    return 2;
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, strange, we could not close the configuration file\n");
    return ERR_CONFIG_FILE;
  }    
  return 0;
}
