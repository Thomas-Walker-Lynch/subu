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

  /*--------------------------------------------------------------------------------
  */
  uid_t useruid;
  gid_t usergid;
  {
    #ifdef DEBUG
    dbprintf("dispatching sss_cache -U to clear users\n");
    #endif
    char *command = "/usr/sbin/sss_cache";
    char *argv[3];
    argv[0] = command;
    argv[1] = "-U";
    argv[2] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    int ret = dispatch(argv, envp);
    if(ret == -1){
      fprintf(stderr, "sss_cache failed\n");
      return -1;
    }
  }  

  #ifdef DEBUG
  dbprintf("finished sss_cache without errors\n", username);
  #endif
  return 0;
}
