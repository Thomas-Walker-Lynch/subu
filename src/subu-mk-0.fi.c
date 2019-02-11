/*
  Makes a new subu user.

  According to the man page, we are not alloed to free the buffers made by getpwid().

  1. We have to make the subu first so that we will have subu_uid and subu_gid
     to work with.

  2. Then we add user access via setfacl to masteru's home directory and to
     subuland, so that we have permissions to make the home directory.

  3. Then we create the subu home directory. .. I wonder where the system
     gets the umask for this?  Perhaps we should create the dir, and then change
     the ownership instead?
     
  4. Still as subu, we add facls to our directory to give masteru access.

  ... then finished, good part is that we never need to move back to root.

setfacl -m u:subu:x masteru
setfacl -m u:subu:x masteru/subuland
setfacl -m d:u:masteru:rwX,u:masteru:rwX subu

*/
// without #define get warning: implicit declaration of function ‘seteuid’/‘setegid’
#define _GNU_SOURCE   

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include "config.h"
#include "useradd.h"
#include "subu-mk-0.fi.h"

typedef unsigned int uint;

uid_gid subu_mk_0(char *subuname){

  //--------------------------------------------------------------------------------
  // we must be invoked from a user account and be running as root
  uid_t uid = getuid();
  uid_t euid = geteuid();
  gid_t gid = getgid();
  gid_t egid = getegid();
  #ifdef DEBUG
    printf("uid %u, gid %u, euid %u\n", uid, gid, euid);
  #endif
  if( uid == 0 || euid != 0 ){
    fprintf(stderr, "this program must be run setuid root from a user account\n");
    return ERR_SETUID_ROOT;
  }

  //--------------------------------------------------------------------------------
  // who are these people anyway?
  size_t subuname_len;
  char *masteru_name;
  {
    // subuname is the first argument passed in
    // verify that subuname is legal!  --> code goes here ...  
    subuname_len = strlen(subuname);
    struct passwd *masteru_pw_record_pt = getpwuid(uid);
    masteru_name = materu_pw_record_pt->pw_name;
  }
  #ifdef DEBUG
  printf("masteru_name: %s\n", masteru_name);
  #endif

  //--------------------------------------------------------------------------------
  // build the subuland path
  char *masteru_home;
  char *subuland;
  size_t subuland_len;
  {
    masteru_home = passwd_record_pt->pw_dir;
    size_t masteru_home_len = strlen(masteru_home);
    if( masteru_home_len == 0 || masteru_home[0] == '(' ){
      fprintf(stderr,"strange, %s has no home directory\n", masteru_name);
      return ERR_BAD_MASTERU_HOME;
    }
    char *subuland_extension = "/subuland/";
    size_t subuland_extension_len = strlen(subuland_extension);
    subuland = (char *)malloc( masteru_home_len + subuland_extension_len + 1 );
    strcpy(subuland, masteru_home);
    strcpy(subuland + masteru_home_len, subuland_extension);
    subuland_len = masteru_home_len + subuland_extension_len + subuname_len;
  }
  #ifdef DEBUG
  printf("The path to subuland: \"%s\"\n", subuland);
  #endif

  //--------------------------------------------------------------------------------
  // Just because masteru_home is referenced in /etc/passwd does not mean it exists.
  // We also require that the subuland sub directory exists.
  {
    struct stat st;
    if( stat(masteru_home, &st) == -1) {
      fprintf(stderr, "Strange, masteru home does not exist, \"%s\".", masteru_home);
      return ERR_NOT_EXIST_MASTERU_HOME;
    }
    if( stat(subuland, &st) == -1) {
      fprintf(stderr, "$masteru_home/subuland not exist");
      return ERR_NOT_EXIST_SUBULAND;
    }
  }

  //--------------------------------------------------------------------------------
  // the name for the subu home directory, which is subuland/subuname
  char subuhome;
  {
    subuhome_len = subuland_len + subuname_len;
    subuhome = (char *)malloc(subuhome_len + 1);
    strcpy (subuhome, subuland);
    strcpy (subuhome + subuland_len, subuname);
  }
  #ifdef DEBUG
  printf("subuhome: %s\n", subuhome);
  #endif

  /*--------------------------------------------------------------------------------
    make the subservient user account
    We can't make the home directory yet, because we need to add execute access rights to
    masteru home and subuland so that the subu user can cd to subuhome.
           -d, --home-dir HOME_DIR
           The new user will be created using HOME_DIR as the value for the user's login directory. The default
           is to append the LOGIN name to BASE_DIR and use that as the login directory name. The directory
           HOME_DIR does not have to exist but will not be created if it is missing.
  */
  uid_t subu_uid;
  tid_t subu_gid;
  {
    char *argv[5];
    argv[0] = "/usr/bin/useradd";
    argv[1] = subuname;
    argv[2] = "-d";
    argv[3] = subuhome;
    argv[4] = (char *) NULL;
    char *envp[1];
    envp[0] (char *) NULL;
    struct ret u_ret = useradd(argv, envp);
    if(ret.error){
      fprintf(stderr, "%u useradd failed\n", ret.error)
      return ERR_FAILED_USERADD;
    }
    subu_uid = user_ret.pw_record->pw_uid;
    subu_gid = user_ret.pw_record->pw_gid;
  }  
  
  //--------------------------------------------------------------------------------
  // give subu x access to masteru and subuland
  {
    char access[2 + subuname_len + 2 + 1];
    strcpy(access, "u:");
    strcpy(access + 2, subuname);
    strcpy(access + 2 + subuname_len, ":x");

    char *argv[];
    argv[0] = "/usr/bin/setfacl";
    argv[1] = "-m";
    argv[2] = access;
    argv[3] = masteru_home
      
  }

  // subu sets acls to allow masteru access
  
  free(subuland);
  free(subuhome);
  return 0;
}

/*
  // change to subu space
  if( seteuid(subu_uid) == -1 || setegid(subu_gid) == -1 ){ // we are root so this should never happen
    fprintf(stderr,"Strangely, root could not seteuid/setegid to %s\n", subuname);
    return ERR_FAILED_MKDIR_SUBU;
  }

*/
