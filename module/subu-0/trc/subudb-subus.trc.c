#tranche subudb-subus.cli.c
/*
Set or get a new maximum subu number. Currently doesn't do the setting part.

*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "common.lib.h"
#include "subudb.lib.h"
#include "subu.lib.h"

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

  Da subu_arr;
  Da *subu_arrp = &subu_arr;
  rc = subudb_Masteru_Subu_get_subus(db, masteru_name, subu_arrp);
  if( rc == SQLITE_OK ){
    char *pt = subu_arrp->base;
    subudb_subu_element *pt1;
    while( pt < subu_arrp->end ){
      pt1 = (subudb_subu_element *)pt;
      printf("%s %s\n", pt1->subuname, pt1->subu_username);
    pt += subu_arrp->element_size;
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
#tranche-end
