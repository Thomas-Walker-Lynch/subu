/*
get the username from the config file
for testing subu_Masteru_Subu_get_user

*/
#include "subu-rel-get.cli.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){

  if(argc != 3){
    fprintf(stderr, "usage: %s masteru_name subuname\n", argv[0]);
    return SUBU_ERR_ARG_CNT;
  }

  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(Config_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "could not open configuration file \"%s\"\n", Config_File);
    return SUBU_ERR_CONFIG_FILE;
  }

  char *masteru_name = argv[1];
  char *subuname = argv[2];
  char *subu_username;

  int ret = subu_Masteru_Subu_get(db, masteru_name, subuname, &subu_username);
  if( ret != SQLITE_DONE ){
    fprintf(stderr, "subu_Masteru_Subu_get indicates failure by returning %d\n",ret);
    fprintf(stderr, "sqlite3 issues message, %s\n", sqlite3_errmsg(db));
    return SUBU_ERR_CONFIG_FILE;
  }
  ret = sqlite3_close(db)
  if( ret != SQLITE_OK ){
    fprintf(stderr, "sqlite3_close(db) indicates failure by returning %d\n",ret);
    fprintf(stderr, "sqlite3 issues message: %s\n", sqlite3_errmsg(db));
    return SUBU_ERR_CONFIG_FILE;
  }    
  return 0;
}
