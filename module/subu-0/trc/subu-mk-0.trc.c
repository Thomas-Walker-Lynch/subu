#tranche subu-mk-0.cli.c
/*
  subu-mk-0 command

*/
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
    fprintf(stderr, "error when opening db, %s\n", DB_File);
    fprintf(stderr, "sqlite3 says: %s\n", sqlite3_errmsg(db));
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
#tranche-end
