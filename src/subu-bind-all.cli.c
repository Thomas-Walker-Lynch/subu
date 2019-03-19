/*
mount all the subu user directories into master's subuland
uses unmount to undo this

*/
#include "subu-bind-all.cli.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
  if( argc != 1){
    fprintf(stderr, "%s does not take arguments\n",argv[0]);
    return SUBU_ERR_ARG_CNT;
  }

  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "could not open db file \"%s\"\n", DB_File);
    return SUBU_ERR_DB_FILE;
  }

  char *mess;
  rc = subu_bind_all(&mess, db);
  if(rc != 0){
    fprintf(stderr, "subu-bind: %s\n", mess);
    return rc;
  }
  return 0;
}
