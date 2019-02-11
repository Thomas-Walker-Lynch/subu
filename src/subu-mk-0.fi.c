/*
  Makes a new subu user.

  According to the man page, we are not alloed to free the buffers made by getpwid().

  1. We have to make the subu first so that we will have subu_uid and subu_gid
     to work with.

  2. Then we add user access via setfacl to masteru's home directory and to
     subu_land, so that we have permissions to make the home directory.

  3. Then we create the subu home directory. .. I wonder where the system
     gets the umask for this?  Perhaps we should create the dir, and then change
     the ownership instead?
     
  4. Still as subu, we add facls to our directory to give masteru access.

  ... then finished, good part is that we never need to move back to root.

setfacl -m u:subu:x masteru
setfacl -m u:subu:x masteru/subu_land
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
#include "subu_mk_0.fi.h"

typedef unsigned int uint;

uid_gid subu_mk_0(char *subu_name){

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
  size_t subu_name_len;
  char *masteru_name;
  {
    // subu_name is the first argument passed in
    // verify that subu_name is legal!  --> code goes here ...  
    subu_name_len = strlen(subu_name);
    struct passwd *masteru_pw_record_pt = getpwuid(uid);
    masteru_name = materu_pw_record_pt->pw_name;
  }
  #ifdef DEBUG
  printf("masteru_name: %s\n", masteru_name);
  #endif

  //--------------------------------------------------------------------------------
  // build the subu_land path
  char *subu_land;
  size_t subu_land_len;
  {
    char *masteru_home_dir = passwd_record_pt->pw_dir;
    size_t masteru_home_dir_len = strlen(masteru_home_dir);
    if( masteru_home_dir_len == 0 || masteru_home_dir[0] == '(' ){
      fprintf(stderr,"strange, %s has no home directory\n", masteru_name);
      return ERR_BAD_MASTERU_HOME;
    }
    char *subu_land_extension = "/subu_land/";
    size_t subu_land_extension_len = strlen(subu_land_extension);
    // add space in the subu_land buffer to append the subu_name later
    subu_land = (char *)malloc( masteru_home_dir_len + subu_land_extension_len + subu_name_len + 1 );
    strcpy(subu_land, masteru_home_dir);
    strcpy(subu_land + masteru_home_dir_len, subu_land_extension);
    subu_land_len = masteru_home_dir_len + subu_land_extension_len + subu_name_len;
  }
  #ifdef DEBUG
  printf("The path to subu_land: \"%s\"\n", subu_land);
  #endif

  //--------------------------------------------------------------------------------
  // Just because masteru_home_dir is referenced in /etc/passwd does not mean it exists.
  // We also require that the subu_land sub directory exists.
  {
    struct stat st;
    if( stat(masteru_home_dir, &st) == -1) {
      fprintf(stderr, "Strange, masteru home does not exist, \"%s\".", masteru_home_dir);
      return ERR_NOT_EXIST_MASTERU_HOME;
    }
    if( stat(subu_land, &st) == -1) {
      fprintf(stderr, "$masteru_home/subu_land not exist");
      return ERR_NOT_EXIST_SUBU_LAND;
    }
  }

  //--------------------------------------------------------------------------------
  // the name for the subu home directory, which is subu_land/subu_name
  strcpy (subu_land + subu_land_len, subu_name); // we had left room in the subu_land buffer for subu_name
  char *subu_home_path = subu_land; // the buffer now holds the subu_home_path, so we call it as such
  #ifdef DEBUG
  printf("subu home: %s\n", subu_home_path);
  #endif

  //--------------------------------------------------------------------------------
  // make the subservient user account
  uid_t subu_uid;
  tid_t subu_gid;
  {
    char *argv[5];
    argv[0] = command;
    argv[1] = subu_name;
    argv[2] = "-d";
    argv[3] = subu_home_path;
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
  // Set acls on the subu home directory
  

  // subu sets acls to allow masteru access
  
  return 0;
}

/*
  // change to subu space
  if( seteuid(subu_uid) == -1 || setegid(subu_gid) == -1 ){ // we are root so this should never happen
    fprintf(stderr,"Strangely, root could not seteuid/setegid to %s\n", subu_name);
    return ERR_FAILED_MKDIR_SUBU;
  }

*/
