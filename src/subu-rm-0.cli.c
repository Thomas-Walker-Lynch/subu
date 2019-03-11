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

  sqlite3 *db;
  {
    int ret = sqlite3_open_v2(Config_File, &db, SQLITE_OPEN_READWRITE, NULL);
    if( ret != SQLITE_OK ){
      fprintf(stderr, "error exit, could not open configuration file \"%s\"\n", Config_File);
      return SUBU_ERR_CONFIG_FILE;
    }}

  {
    char *mess;
    int ret = subu_mk_0(&mess, db, subuname);
    subu_err("subu_mk_0", ret, mess);
    free(mess);
    return ret;
  }
}
