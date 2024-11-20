#tranche common.lib.c
#include "common.lib.h"

#tranche common.lib.h
  typedef unsigned int uint;
  /*
    Fedora 29's sss_cache is checking the inherited uid instead of the effective
    uid, so setuid root scripts will fail when calling sss_cache.

    Fedora 29's 'useradd' calls sss_cache, and useradd is called by our setuid root 
    program subu-mk-0.
  */
  #define BUG_SSS_CACHE_RUID 1
#tranche-end

#tranche common.lib.h
  // extern char *DB_File;
  extern char DB_File[];
  extern uint Subuhome_Perms;
  extern uint First_Max_Subunumber;
  extern char Subuland_Extension[];
#tranche-end
// char *DB_File = "/etc/subudb";
char DB_File[] = "subudb";
uint Subuhome_Perms = 0700;
uint First_Max_Subunumber = 114;
char Subuland_Extension[] = "/subuland/";

#tranche-end
