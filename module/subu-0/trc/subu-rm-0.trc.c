#tranche subu-rm-0.cli.c
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

  sqlite3 *db;
  {
    int ret = sqlite3_open_v2(DB_File, &db, SQLITE_OPEN_READWRITE, NULL);
    if( ret != SQLITE_OK ){
      fprintf(stderr, "error exit, could not open db file \"%s\"\n", DB_File);
      return SUBU_ERR_DB_FILE;
    }}

  {
    char *mess=0;
    int ret = subu_rm_0(&mess, db, subuname);
    subu_err("subu_rm_0", ret, mess);
    free(mess);
    return ret;
  }
}
#tranche-end
