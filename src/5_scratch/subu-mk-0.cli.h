/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <stdbool.h>
#include <errno.h>
#include <sqlite3.h>
typedef struct subu_mk_0_ctx subu_mk_0_ctx;
void subu_mk_0_ctx_free(struct subu_mk_0_ctx *ctxp);
void subu_mk_0_mess(struct subu_mk_0_ctx *ctxp);
struct subu_mk_0_ctx *subu_mk_0(sqlite3 *db,char *subuname);
typedef unsigned int uint;
struct subu_mk_0_ctx {
  char *name;
  char *subuland;
  char *subuhome;
  char *subu_username;
  bool free_aux;
  char *aux;
  uint err;
};
extern char config_file[];
