/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subudb-number.cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char **argv){

  if( argc < 2 || argc > 3){
    fprintf(stderr, "usage: %s masteru_name [n]\n",argv[0]);
    return SUBU_ERR_ARG_CNT;
  }
  char *masteru_name = argv[1];

  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open db file\n");
    sqlite3_close(db);
    return SUBU_ERR_DB_FILE;
  }

  // then arg[2] holds a number to set the max to
  if(argc == 3){ 
    long int i = strtol(argv[2], NULL, 10);
    if( i < 0 ){
      fprintf(stderr, "n must be positive\n");
      sqlite3_close(db);
      return SUBU_ERR_N;
    }
    if( i > INT_MAX ){
      fprintf(stderr, "n is too big, max supported by this program is %d\n", INT_MAX);
      sqlite3_close(db);
      return SUBU_ERR_N;
    }
    int n = i;
    subudb_number_set(db, masteru_name, n);
  }

  // read and print the current max
  int n;
  rc = subudb_number_get(db, masteru_name, &n);
  if( rc == SQLITE_DONE ){
    printf("%d\n", n);
  }else{
    fprintf(stderr, "lookup failed %s\n", sqlite3_errmsg(db));
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
