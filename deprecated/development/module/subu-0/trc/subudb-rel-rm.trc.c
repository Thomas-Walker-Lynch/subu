#tranche subudb-rel-rm.cli.c
/*
puts a relation in the masteru/subu table

*/
#include <stdio.h>
#include <stdlib.h>
#include "common.lib.h"
#include "subudb.lib.h"
#include "subu.lib.h"

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
    int ret = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
    if( ret != SQLITE_OK ){
      fprintf(stderr, "could not open db file \"%s\"\n", DB_File);
      return SUBU_ERR_DB_FILE;
    }}

  int ret = subudb_Masteru_Subu_rm(db, masteru_name, subuname, subu_username);
  if( ret != SQLITE_DONE ){
    fprintf(stderr, "subudb_Masteru_Subu_rm indicates failure by returning %d\n",ret);
    fprintf(stderr, "sqlite3 issues message, %s\n", sqlite3_errmsg(db));
    printf("put failed\n");
    return 2;
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
