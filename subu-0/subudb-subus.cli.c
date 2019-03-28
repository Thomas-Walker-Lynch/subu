/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include "subudb-subus.cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char **argv){

  if( argc != 2){
    fprintf(stderr, "usage: %s masteru_name\n",argv[0]);
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

  subudb_subu_element *sa;
  subudb_subu_element *sa_end;
  rc = subudb_Masteru_Subu_get_subus(db, masteru_name, &sa, &sa_end);
  if( rc == SQLITE_OK ){
    subudb_subu_element *pt = sa;
    while( pt != sa_end ){
      printf("%s %s\n", pt->subuname, pt->subu_username);
    pt++;
    }
    rc = sqlite3_close(db);
    if( rc != SQLITE_OK ){
      fprintf(stderr, "when closing db, %s\n", sqlite3_errmsg(db));
      return SUBU_ERR_DB_FILE;
    }    
    return 0;
  }
  fprintf(stderr, "lookup failed %s\n", sqlite3_errmsg(db));
  sqlite3_close(db);
  return SUBU_ERR_DB_FILE;

}
