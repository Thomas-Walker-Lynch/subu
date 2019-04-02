#tranche subudb-rel-get.cli.c
/*
get the username from the db file
for testing subudb_Masteru_Subu_get_subu_username

*/
#include <stdio.h>
#include <stdlib.h>
#include "common.lib.h"
#include "subudb.lib.h"
#include "subu.lib.h"

int main(int argc, char **argv){

  if(argc != 3){
    fprintf(stderr, "usage: %s masteru_name subuname\n", argv[0]);
    return SUBU_ERR_ARG_CNT;
  }

  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "could not open db file \"%s\"\n", DB_File);
    return SUBU_ERR_DB_FILE;
  }

  char *masteru_name = argv[1];
  char *subuname = argv[2];
  char *subu_username;

  int ret = subudb_Masteru_Subu_get_subu_username(db, masteru_name, subuname, &subu_username);
  if( ret != SQLITE_DONE ){
    fprintf(stderr, "subudb_Masteru_Subu_get_subu_username indicates failure by returning %d\n",ret);
    fprintf(stderr, "sqlite3 issues message, %s\n", sqlite3_errmsg(db));
    return SUBU_ERR_DB_FILE;
  }
  ret = sqlite3_close(db);
  if( ret != SQLITE_OK ){
    fprintf(stderr, "sqlite3_close(db) indicates failure by returning %d\n",ret);
    fprintf(stderr, "sqlite3 issues message: %s\n", sqlite3_errmsg(db));
    return SUBU_ERR_DB_FILE;
  }    
  return 0;
}
#tranche-end
