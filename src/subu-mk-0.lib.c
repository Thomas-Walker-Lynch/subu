/*
  Makes a new subu user.

  According to the man page, we are not alloed to free the memory allocated by getpwid().


setfacl -m d:u:masteru:rwX,u:masteru:rwX subuname

*/
// without this #define we get the warning: implicit declaration of function ‘seteuid’/‘setegid’
#define _GNU_SOURCE   
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include "dispatch.lib.h"
#include "dispatch_useradd.lib.h"
#include "subu-mk-0.lib.h"

typedef unsigned int uint;

int subu_mk_0(char *subuname){

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Checking we are running from a user and are setuid root.\n");
  #endif
  uid_t uid = getuid();
  uid_t euid = geteuid();
  gid_t gid = getgid();
  gid_t egid = getegid();
  #ifdef DEBUG
  dbprintf("uid %u, gid %u, euid %u egid %u\n", uid, gid, euid, egid);
  #endif
  if( uid == 0 || euid != 0 ){
    fprintf(stderr, "this program must be run setuid root from a user account\n");
    return ERR_SETUID_ROOT;
  }

  //--------------------------------------------------------------------------------
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
  // Just because masteru_home is referenced in /etc/passwd does not mean it exists.
  // We also require that the subuland sub directory exists.
  {
    #ifdef DEBUG
    dbprintf("checking that masteru_home and subuland exist\n");
    #endif
    struct stat st;
    if( stat(masteru_home, &st) == -1) {
      fprintf(stderr, "Strange, masteru home does not exist, \"%s\".", masteru_home);
      free(subuland);
      return ERR_NOT_EXIST_MASTERU_HOME;
    }
    if( stat(subuland, &st) == -1) {
      fprintf(stderr, "$masteru_home/subuland/ does not exist");
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
      perror("subu_mk_0");
      free(subuland);
      return ERR_MALLOC;
    }
    strcpy (subuhome, subuland);
    strcpy (subuhome + subuland_len, subuname);
  }
  #ifdef DEBUG
  dbprintf("subuhome %s\n", subuhome);
  #endif

  /*--------------------------------------------------------------------------------
    We need to add execute access rights to masteru home and subuland so that
    the subu user can cd to subuhome.

    Ok to specify the new home directory in useradd, because it doesn't make it.
    From the man page:

           -d, --home-dir HOME_DIR The new user will be created using HOME_DIR
           as the value for the user's login directory. ... The directory HOME_DIR
           does not have to exist but will not be created if it is missing.
  */
  uid_t subuuid;
  gid_t subugid;
  {
    #ifdef DEBUG
    dbprintf("making subu\n");
    #endif
    char *command = "/usr/sbin/useradd";
    char *argv[5];
    argv[0] = command;
    argv[1] = subuname;
    argv[2] = "-d";
    argv[3] = subuhome;
    argv[4] = (char *) NULL;
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
    #ifdef DEBUG
    dbprintf("subu made without errors\n", command, subuname, subuhome);
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
      perror("subu_mk_0");
      free(subuland);
      free(subuhome);
      return ERR_MK_SUBUHOME;
    }
    ret = chown(subuhome, subuuid, subugid);
    if( ret == -1 ){
      perror("subu_mk_0");
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
