/*
  Makes a new subu user.

  According to the man page, we are not allowed to free the memory allocated by getpwid().

  masteru is the user who ran this script. subu is the user being created.

  subu-mk-0 is a setuid root script. 

  see also 3_doc/subu-mk-0.txt

*/

// without this #define we get the warning: implicit declaration of function ‘seteuid’/‘setegid’
#define _GNU_SOURCE   

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "dispatch_f.lib.h"
#include "dispatch_exec.lib.h"
#include "dispatch_useradd.lib.h"
#include "subu-mk-0.lib.h"

typedef unsigned int uint;

/*
  Fedora 29's sss_cache is checking the inherited uid instead of the effective
  uid, so setuid root scripts will fail when calling sss_cache. Fedora 29's
  'useradd'  calls sss_cache.
 
*/
#define BUG_SSS_CACHE_RUID 1

// will be called through dispatch_f_as masteru
int masteru_makes_subuhome(void *arg){
  char *subuhome = (char *) arg;
  if( mkdir( subuhome, 0700 ) == -1 ){
    perror("masteru_makes_subuhome");
    return -1;
  }
  return 0;
}

int subu_mk_0(char *subuname){

  char *perror_src = "subu_mk_0";

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Checking that we are running from a user and are setuid root.\n");
  #endif
  uid_t masteru_uid = getuid();
  gid_t masteru_gid = getgid();
  uid_t set_euid = geteuid();
  gid_t set_egid = getegid();
  #ifdef DEBUG
  dbprintf("masteru_uid %u, masteru_gid %u, set_euid %u set_egid %u\n", masteru_uid, masteru_gid, set_euid, set_egid);
  #endif
  if( masteru_uid == 0 || set_euid != 0 ){
    fprintf(stderr, "this program must be run setuid root from a user account\n");
    return ERR_SETUID_ROOT;
  }

  //--------------------------------------------------------------------------------
  // recover the masteru user name from /etc/passwd

  // char *subuname was passed in as an argument
  size_t subuname_len;
  char *masteru_name;
  size_t masteru_name_len;
  struct passwd *masteru_pw_record_pt;
  {
    #ifdef DEBUG
    dbprintf("looking up masteru_name (i.e. uid %u) in /etc/passwd\n", masteru_uid);
    #endif
    // subuname is the first argument passed in
    // verify that subuname is legal!  --> code goes here ...  
    subuname_len = strlen(subuname);
    masteru_pw_record_pt = getpwuid(masteru_uid);
    masteru_name = masteru_pw_record_pt->pw_name;
    masteru_name_len = strlen(masteru_name);
  }
  #ifdef DEBUG
  dbprintf("subuname \"%s\" masteru_name: \"%s\"\n", subuname, masteru_name);
  #endif

  //--------------------------------------------------------------------------------
  char *masteru_home;
  size_t masteru_home_len;
  char *subuland;
  size_t subuland_len;
  {
    #ifdef DEBUG
    dbprintf("building the subuland path\n");
    #endif
    masteru_home = masteru_pw_record_pt->pw_dir;
    masteru_home_len = strlen(masteru_home);
    if( masteru_home_len == 0 || masteru_home[0] == '(' ){
      fprintf(stderr,"strange, %s has no home directory\n", masteru_name);
      return ERR_BAD_MASTERU_HOME;
    }
    char *subuland_extension = "/subuland/";
    size_t subuland_extension_len = strlen(subuland_extension);
    subuland = (char *)malloc( masteru_home_len + subuland_extension_len + 1 );
    strcpy(subuland, masteru_home);
    strcpy(subuland + masteru_home_len, subuland_extension);
    subuland_len = masteru_home_len + subuland_extension_len;
  }
  #ifdef DEBUG
  dbprintf("masteru_home: \"%s\"\n", masteru_home);
  dbprintf("The path to subuland: \"%s\".\n", subuland);
  #endif

  //--------------------------------------------------------------------------------
  // the path for the subu home directory, which is $(masteru_home)/subuland/subuname
  char *subuhome;
  size_t subuhome_len;
  {
    #ifdef DEBUG
    dbprintf("making the path for subuhome\n");
    #endif
    subuhome_len = subuland_len + subuname_len;
    subuhome = (char *)malloc(subuhome_len + 1);
    if( !subuhome ){
      perror(perror_src);
      free(subuland);
      return ERR_MALLOC;
    }
    strcpy (subuhome, subuland);
    strcpy (subuhome + subuland_len, subuname);
  }
  #ifdef DEBUG
  dbprintf("subuhome: \"%s\"\n", subuhome);
  #endif

  //--------------------------------------------------------------------------------
  // as masteru, create the subuhome directory
  // if subuland does not exist, or if masteru doesn't have permissions, this will fail
  //
  {
    #ifdef DEBUG
    dbprintf("making subuhome\n");
    #endif
    struct stat st;
    if( stat(subuhome, &st) != -1 ){
      fprintf(stderr, "an file system object already exists at the subu home directory path\n");
      free(subuland);
      free(subuhome);
      return ERR_MK_SUBUHOME;
    }
    int ret = dispatch_f_euid_egid
      (
       "masteru_makes_subuhome", 
       masteru_makes_subuhome, 
       (void *)subuhome,
       masteru_uid, 
       masteru_gid
       );
    if( ret == -1 ){
      free(subuland);
      free(subuhome);
      return ERR_FAILED_MKDIR_SUBU;
    }
  }
  #ifdef DEBUG
  dbprintf("made directory \"%s\"\n", subuhome);
  #endif

  /*--------------------------------------------------------------------------------
    Make the subservient user, i.e. the subu

    Ok to specify the new home directory in useradd, because it doesn't make it.
    From the man page:

           -d, --home-dir HOME_DIR The new user will be created using HOME_DIR
           as the value for the user's login directory. ... The directory HOME_DIR
           does not have to exist but will not be created if it is missing.

  Actually Fedora 29's 'useradd' is making the directory even when -d  is specified.
  Adding the -M option suppresses it.
  */
  {
    #ifdef DEBUG
      dbprintf("making user \"%s\"\n", subuname);
    #endif
    #if BUG_SSS_CACHE_RUID
      #ifdef DEBUG
        dbprintf("setting inherited real uid to 0 to accomodate SSS_CACHE UID BUG\n");
      #endif
      if( setuid(0) == -1 ){
        perror(perror_src);
        free(subuland);
        free(subuhome);
        return ERR_BUG_SSS;
      }
    #endif
    char *command = "/usr/sbin/useradd";
    char *argv[6];
    argv[0] = command;
    argv[1] = subuname;
    argv[2] = "-d";
    argv[3] = subuhome;
    argv[4] = "-M";
    argv[5] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    struct dispatch_useradd_ret_t ret;
    ret = dispatch_useradd(argv, envp);
    if(ret.error){
      fprintf(stderr, "%u useradd failed\n", ret.error);
      free(subuland);
      free(subuhome);
      return ERR_FAILED_USERADD;
    }
    #ifdef DEBUG
    dbprintf("useradd finished\n");
    #endif
  }  
  
  //--------------------------------------------------------------------------------
  {
    #ifdef DEBUG
    dbprintf("give subu x access to masteru home and subuland, and give it rwx to its home\n");
    #endif
    char *command = "/usr/bin/setfacl";
    char access[strlen("u:") + subuname_len + strlen(":x") + 1  + strlen("rw")];
    strcpy(access, "u:");
    strcpy(access + 2, subuname);
    strcpy(access + 2 + subuname_len, ":x");
    char *argv[5];
    argv[0] = command;
    argv[1] = "-m";
    argv[2] = access;
    argv[3] = masteru_home;
    argv[4] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    if( dispatch_exec(argv, envp) == -1 ){
      fprintf(stderr, "'setfacl -m u:%s:x %s' returned an error.\n", subuname, masteru_home);
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
    argv[3] = subuland;
    if( dispatch_exec(argv, envp) == -1 ){
      fprintf(stderr, "'setfacl -m u:%s:x %s' returned an error.\n", subuname, subuland);
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
    strcpy(access + 2 + subuname_len, ":rwx");
    argv[3] = subuhome;
    if( dispatch_exec(argv, envp) == -1 ){
      fprintf(stderr, "'setfacl -m u:%s:rwx %s' returned an error.\n", subuname, subuhome);
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
    #ifdef DEBUG
    dbprintf("subu can now cd to subuhome\n");
    #endif
  }

  //--------------------------------------------------------------------------------
  {  
    #ifdef DEBUG
    dbprintf("give masteru default access to the subuhome\n");
    #endif
    char *command = "/usr/bin/setfacl";
    char access[strlen("d:u:") + masteru_name_len + strlen(":rwX") + 1];
    strcpy(access, "d:u:");
    strcpy(access + 4, masteru_name);
    strcpy(access + 4 + masteru_name_len, ":rwX");
    char *argv[5];
    argv[0] = command;
    argv[1] = "-m";
    argv[2] = access;
    argv[3] = subuhome;
    argv[4] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    if( dispatch_exec(argv, envp) == -1 ){
      fprintf
        (
         stderr,
         "'setfacl -$ -m d:u:%s:rwX %s' returned an error.\n",
         masteru_name,
         masteru_name,
         subuhome
         );
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
    #ifdef DEBUG
    dbprintf("masteru now has default access\n");
    #endif
  }

  #ifdef DEBUG
  dbprintf("finished subu-mk-0(%s)\n", subuname);
  #endif
  free(subuland);
  free(subuhome);
  return 0;
}
