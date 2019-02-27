/*
  subu-mk-0 command

*/
#include "subu-mk-0.lib.h"
#include <stdio.h>

int main(int argc, char **argv){
  char *command = argv[0];
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", command);
    return ERR_SUBU_MK_0_ARG_CNT;
  }
  char *subuname = argv[1];

  int ret;
  sqlite3 *db;
  ret = sqlite3_open_v2(config_file, &db, SQLITE_OPEN_READWRITE, NULL);
  if( ret != SQLITE_OK ){
    fprintf(stderr, "error exit, could not open configuration file\n");
    return ERR_CONFIG_FILE;
  }

  subu_mk_0_ctx *ctxp = subu_mk_0(sqlite3 *db, subuname);
  subu_mk_0_mess(ctxp);
  int err = ctxp->err;
  subu_mk_0_ctx_free(ctxp);
  return err;
}
