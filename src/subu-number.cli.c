/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subu-number.cli.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){

  if( argc > 2 ){
    fprintf(stderr, "usage: %s [n]\n",argv[0]);
    return SUBU_ERR_ARG_CNT;
  }

  int rc;
  sqlite3 *db;
  rc = sqlite3_open_v2(Config_File, &db, SQLITE_OPEN_READWRITE, NULL);
  if( rc != SQLITE_OK ){
    sqlite3_close(db);
    fprintf(stderr, "error exit, could not open configuration file\n");
    return SUBU_ERR_CONFIG_FILE;
  }

  // then arg[1] holds a number to set the max to
  if(argc == 2){ 
    long int i = strlol(argc[1], NULL, 10);
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
    subu_number_set(db, n);
  }

  // read and print the current max
  char *n;
  rc = subu_number_get(db, &n);
  if( rc == SQLITE_DONE ){
    printf("%s\n", n);
  }else{
    sqlite3_close(db);
    return SUBU_ERR_CONFIG_FILE;
  }
  if( sqlite3_close(db) != SQLITE_OK ){
    fprintf(stderr, "error exit, strange, we could not close the configuration file\n");
    return SUBU_ERR_CONFIG_FILE;
  }    
  return 0;

}
