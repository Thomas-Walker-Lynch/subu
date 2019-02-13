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
#include "sss_cache.lib.h"

typedef unsigned int uint;

int user_mk(char *username){

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
    return -1;
  }
  #ifdef DEBUG
  dbprintf("yes, uid is not zero, and euid is zero, so we are setuid to the root user.\n");
  #endif

  //--------------------------------------------------------------------------------
  char *home;
  size_t home_len;
  {
    #ifdef DEBUG
    dbprintf("making the home dir path\n");
    #endif
    char *prefix = "/home/";
    home_len = strlen(prefix) + strlen(username);
    home = (char *)malloc(home_len + 1);
    if( !home ){
      perror("sss_cache");
      return -1;
    }
    strcpy (home, prefix);
    strcpy (home + strlen(prefix), username);
  }
  #ifdef DEBUG
  dbprintf("home dir path: \"%s\"\n", home);
  #endif

  /*--------------------------------------------------------------------------------
    note this from the man page:

           -d, --home-dir HOME_DIR The new user will be created using HOME_DIR
           as the value for the user's login directory. ... The directory HOME_DIR
           does not have to exist but will not be created if it is missing.
  */
  uid_t useruid;
  gid_t usergid;
  {
    #ifdef DEBUG
    dbprintf("dispatching useradd to create the user\n");
    #endif
    char *command = "/usr/sbin/useradd";
    char *argv[5];
    argv[0] = command;
    argv[1] = username;
    argv[2] = "-d";
    argv[3] = home;
    argv[4] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    int ret = dispatch(argv, envp);
    if(ret == -1){
      fprintf(stderr, "useradd failed\n");
      return -1;
    }
    struct passwd *pw_record = getpwnam(username);
    if( pw_record == NULL ){
      fprintf(stderr,"getpwnam failed after useradd for username, %s\n", username);
    }
    useruid = pw_record->pw_uid;
    usergid = pw_record->pw_gid;
  }  

  //--------------------------------------------------------------------------------
  // create home directory
  //   we have our reasons for doing this second (setting facls in different places)
  {
    #ifdef DEBUG
    dbprintf("mkdir(%s, 0x0700)\n", home);
    #endif
    int ret = mkdir(home, 0x0700);
    if( ret == -1 ){
      perror("sss_cache");
      return -1;
    }
    ret = chown(home, useruid, usergid);
    if( ret == -1 ){
      perror("sss_cache");
      return -1;
    }
  }

  #ifdef DEBUG
  dbprintf("finished sss_cache without errors\n", username);
  #endif
  return 0;
}
