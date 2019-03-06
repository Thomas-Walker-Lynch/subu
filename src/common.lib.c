
#include "common.lib.h"

#if INTERFACE
typedef unsigned int uint;
/*
  Fedora 29's sss_cache is checking the inherited uid instead of the effective
  uid, so setuid root scripts will fail when calling sss_cache.

  Fedora 29's 'useradd' calls sss_cache, and useradd is called by our setuid root 
  program subu-mk-0.
*/
#define BUG_SSS_CACHE_RUID 1
#endif

//  char *config_file = "/etc/subu.db";
char config_file[] = "subu.db";
uint subuhome_perms = 0700;
uint first_max_subu_number = 114;
