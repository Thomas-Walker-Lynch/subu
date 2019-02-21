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
#include "dispatch.lib.h"
#include "dispatch_useradd.lib.h"
#include "dispatch_f.lib.h"
#include "subu-mk-0.lib.h"

typedef unsigned int uint;

/*
  Fedora 29's sss_cache is checking the inherited uid instead of the effective
  uid, so setuid root scripts will fail when calling sss_cache. Fedora 29's
  'useradd'  calls sss_cache.
 
*/
#define BUG_SSS_CACHE_RUID 1

int subu_mk_0(char *subuname){

  char *perror_src = "subu_mk_0";

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Checking that we are running from a user and are setuid root.\n");
  #endif
  uid_t master_uid = getuid();
  gid_t master_gid = getgid();
  uid_t set_euid = geteuid();
  gid_t set_egid = getegid();
  #ifdef DEBUG
  dbprintf("master_uid %u, master_gid %u, set_euid %u set_egid %u\n", master_uid, master_gid, set_euid, set_egid);
  #endif
  if( master_uid == 0 || set_euid != 0 ){
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
    dbprintf("looking up masteru_name (i.e. uid %u) in /etc/passwd\n",uid);
    #endif
    // subuname is the first argument passed in
    // verify that subuname is legal!  --> code goes here ...  
    subuname_len = strlen(subuname);
    masteru_pw_record_pt = getpwuid(uid);
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
  // Just because masteru_home is referenced in /etc/passwd does not mean it exists,
  // and does not mean that masteru owns it or has 'x' privileges.
  // We also require that the subuland sub directory exists.
  {
    #ifdef DEBUG
    dbprintf("checking that masteru_home and subuland exist\n");
    #endif
    struct stat st;
    int stat_ret;

    stat_ret = stat(masteru_home, &st);
    if( stat_ret == -1 ){
      fprintf(stderr, "masteru home directory does not exist, \"%s\".", masteru_home);
      free(subuland);
      return ERR_NOT_EXIST_MASTERU_HOME;
    }else if( !S_ISDIR(st.st_mode) ) {
      fprintf(stderr, "strange masteru home directory is not a directory, \"%s\".", masteru_home);
      free(subuland);
      return ERR_NOT_EXIST_MASTERU_HOME;
    }else if( ){
    }

    stat(subuland, &st);
    if( !S_ISDIR(st.st_mode) ) {
      fprintf(stderr, "$masteru_home/subuland/ directory does not exist");
      free(subuland);
      return ERR_NOT_EXIST_SUBULAND;
    }
  }

  //--------------------------------------------------------------------------------
  // the name for the subu home directory, which is $(masteru_home)/subuland/subuname
  char *subuhome;
  size_t subuhome_len;
  {
    #ifdef DEBUG
    dbprintf("making the name for subuhome\n");
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

  /*--------------------------------------------------------------------------------
    We need to add execute access rights to masteru home and subuland so that
    the subu user can cd to subuhome.

    Ok to specify the new home directory in useradd, because it doesn't make it.
    From the man page:

           -d, --home-dir HOME_DIR The new user will be created using HOME_DIR
           as the value for the user's login directory. ... The directory HOME_DIR
           does not have to exist but will not be created if it is missing.

  Actually Fedora 29's 'useradd' is making the directory even when -d  is specified.
  Adding the -M option supresses it.
  */

  uid_t subuuid;
  gid_t subugid;
  bool subuhome_already_exists = false;
  {
    #ifdef DEBUG
      dbprintf("making subu\n");
    #endif

    #if BUG_SSS_CACHE_RUID
      if( setuid(0) == -1 ){
        perror(perror_src);
        return ERR_BUG_SSS;
      }
    #endif
    struct stat st;
    if( stat(subuhome, &st) != -1 ){
      if( !S_ISDIR(st.st_mode) ) {
        subuhome_already_exists = true;
      }else{
        fprintf(stderr, "Home directory would clobber non-directory object already at %s\n", subuhome);
        return ERR_MK_SUBUHOME;
      }}
     
    char *command = "/usr/sbin/useradd";
    char *argv[6];
    argv[0] = command;
    argv[1] = subuname;
    argv[2] = "-d";
    argv[3] = subuhome;
    argv[4] = -M
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
    subuuid = ret.pw_record->pw_uid;
    subugid = ret.pw_record->pw_gid;
    bool err_mk_subuhome = false;
    if( !subuhome_already_exists && stat(subuhome, &st) != -1 ){
      if( S_ISDIR(st.st_mode) ){
        #if !BUG_USERADD_ALWAYS_MKHOME
        err_mk_subuhome = true;
        fprintf(stderr, "useradd -d unexpectedly created the subuhome, will delete that now\n");
        #endif
        if( rmdir(subuhome) == -1 ){
          err_mk_subuhome = true;
          fprintf(stderr, "could not delete the subuhome created by useradd, bailing\n");
          return ERR_MK_SUBUHOME;
        }
      }else{
        err_mk_subuhome = true;
        fprintf(stderr, "useradd, or a parallel running process, has created a non-directory object at subuhome\n");
        return ERR_MK_SUBUHOME;
      }}

    if( err_mk_subuhome )
      fprintf(stderr, "encountered some difficulties when attempging to make subu, you better have a look\n");

    #ifdef DEBUG
    if( err_mk_subuhome )
      dbprintf("useradd finished");
    else
      dbprintf("useradd finished with no errors\n");
    #endif
  }  
  
  //--------------------------------------------------------------------------------
  {
    #ifdef DEBUG
    dbprintf("give subu x access to masteru and subuland\n");
    #endif
    char *command = "/usr/bin/setfacl";
    char access[2 + subuname_len + 2 + 1];
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
    if( dispatch(argv, envp) == -1 ){
      fprintf(stderr, "'setfacl -m u:%s:x %s' returned an error.\n", subuname, masteru_home);
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
    argv[3] = subuland;
    if( dispatch(argv, envp) == -1 ){
      fprintf(stderr, "'setfacl -m u:%s:x %s' returned an error.\n", subuname, subuland);
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
  }

  //--------------------------------------------------------------------------------
  // create subuhome directory
  {
    #ifdef DEBUG
    dbprintf("mkdir(%s, 0x0700)\n", subuhome);
    #endif
    int ret = mkdir(subuhome, 0x0700);
    if( ret == -1 ){
      perror(perror_src);
      free(subuland);
      free(subuhome);
      return ERR_MK_SUBUHOME;
    }
    ret = chown(subuhome, subuuid, subugid);
    if( ret == -1 ){
      perror(perror_src);
      free(subuland);
      free(subuhome);
      return ERR_MK_SUBUHOME;
    }
  }

  //--------------------------------------------------------------------------------
  {  
    #ifdef DEBUG
    dbprintf("give masteru access to the subuhome/...");
    #endif
    char *command = "/usr/bin/setfacl";
    char access[4 + masteru_name_len + 7 + masteru_name_len + 5 + subuhome_len + 1];
    strcpy(access, "d:u:");
    strcpy(access + 4, masteru_name);
    strcpy(access + 4 + masteru_name_len, ":rwX,u:");
    strcpy(access + 4 + masteru_name_len + 7, masteru_name);
    strcpy(access + 4 + masteru_name_len + 7 + masteru_name_len, ":rwX ");
    strcpy(access + 4 + masteru_name_len + 7 + masteru_name_len + 5, subuhome);
    char *argv[6];
    argv[0] = command;
    argv[1] = "-R";  // just in case the dir already existed with stuff in it
    argv[2] = "-m";
    argv[3] = access;
    argv[4] = subuhome;
    argv[5] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    if( dispatch(argv, envp) == -1 ){
      fprintf
        (
         stderr,
         "'setfacl -$ -m d:u:%s:rwX,u:%s:rwX %s' returned an error.\n",
         masteru_name,
         masteru_name,
         subuhome
         );
      free(subuland);
      free(subuhome);
      return ERR_SETFACL;
    }
  }

  #ifdef DEBUG
  dbprintf("finished subu-mk-0(%s) without error\n", subuname);
  #endif
  free(subuland);
  free(subuhome);
  return 0;
}
