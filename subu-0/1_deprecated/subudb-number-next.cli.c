/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subudb-number-next.cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char **argv){

  if( argc != 2 ){
    fprintf(stderr, "usage: %s masteru_name \n",argv[0]);
    return SUBU_ERR_ARG_CNT;
  }
  char *masteru_name = argv[1];
  
  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    sqlite3_close(db);
    fprintf(stderr, "error exit, could not open db file\n");
    return SUBU_ERR_DB_FILE;
  }

  // read and print the current max
  char *mess;
  int n;
  rc = subudb_number_next(db, masteru_name, &n, &mess);
  if( rc == SQLITE_DONE ){
    printf("%d\n", n);
  }else{
    fprintf(stderr, "subudb_number_next indicates failure by returning %d\n",rc);
    fprintf(stderr, "and issues message, %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return SUBU_ERR_DB_FILE;
  }
  rc = sqlite3_close(db);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "when closing db, %s\n", sqlite3_errmsg(db));
    return SUBU_ERR_DB_FILE;
  }    
  return 0;
}
