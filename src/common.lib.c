
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

//  char *Config_File = "/etc/subu.db";
char Config_File[] = "subu.db";
uint Subuhome_Perms = 0700;
uint First_Max_Subu_number = 114;
char Subuland_Extension[] = "/subuland/";
