/*
The config file is maintained in SQLite

Because user names of are of limited length, subu user names are always named _s<number>.
A separate table translates the numbers into the subu names.

The first argument is the biggest subu number in the system, or one minus an 
starting point for subu numbering.

currently a unit converted to base 10 will always fit in a 21 bit buffer.

*/
#include "subu-config.lib.h"

#if INTERFACE
#include <sqlite3.h>
#define ERR_CONFIG_FILE -1
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int schema(sqlite3 *db, uint max_subu_number){
  char max_subu_number_string[32];
  uint max_subu_number_string_len = snprintf(max_subu_number_string, 32, "%u", max_subu_number);
  if( max_subu_number_string_len >= 32 ){
    fprintf(stderr, "error exit, max_subu_number too big to deal with\n");
    return ERR_CONFIG_FILE;
  }
  char sql1[] = "CREATE TABLE Master_Subu(master_name TEXT, subu_number INT, subu_name TEXT); ";
  char sql2[] = "CREATE TABLE Key_Int(key TEXT, value INT); ";

  char sql3_1[] = "INSERT INTO Key_Int VALUES( 'max_subu_number', ";
  char sql3_2[] = " ); ";
  char sql3_len = strlen(sql3_1) + max_subu_number_string_len + strlen(sql3_2) + 1;
  char sql3[sql3_len];
  strcpy(sql3, sql3_1);
  strcpy(sql3 + strlen(sql3_1), max_subu_number_string);
  strcpy(sql3 + strlen(sql3_1) + max_subu_number_string_len, sql3_2);

  char sql[strlen(sql1) + strlen(sql2) + strlen(sql3) + 1];
  strcpy(sql, sql1);
  strcpy(sql + strlen(sql1), sql2);
  strcpy(sql + strlen(sql1) + strlen(sql2), sql3);

  return sqlite3_exec(db, sql, NULL, NULL, NULL);
}

static int subu_number_extract(void *n, int colcnt, char **colvals, char **colnames){
  if(colcnt >= 1){
    sscanf(colvals[0], "%u", n);
    return 0;
  }
  return -1;
}
int subu_number_get(sqlite3 *db, uint *subu_number){
  *subu_number = 0;
  char *sql = 
    "BEGIN TRANSACTION;"
    "UPDATE Key_Int SET value = value + 1 WHERE key = 'max_subu_number';"
    "SELECT value FROM Key_Int WHERE key = 'max_subu_number';"
    "COMMIT;";
  int ret;
  char *errmsg;
  ret = sqlite3_exec(db, sql, subu_number_extract, subu_number, &errmsg);
  if( errmsg ){
    printf("exec failed when accessing max_subu_number: %s\n", errmsg);
    sqlite3_free(errmsg);
  }
  return ret;
}
