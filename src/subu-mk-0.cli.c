/*
  subu-mk-0 command

*/
#include "subu-mk-0.cli.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return SUBU_ERR_ARG_CNT;
  }
  char *subuname = argv[1];

  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open db file\n");
    sqlite3_close(db);
    return SUBU_ERR_DB_FILE;
  }

  char *mess;
  rc = subu_mk_0(&mess, db, subuname);
  if( rc ){
    subu_err("subu_mk_0", rc, mess);
    free(mess);
    sqlite3_close(db);
    return rc;
  }

  rc = sqlite3_close(db);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "when closing db, %s\n", sqlite3_errmsg(db));
    return SUBU_ERR_DB_FILE;
  }    
  return 0;

}
