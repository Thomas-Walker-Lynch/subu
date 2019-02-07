/*
  Makes a new subu user.

  Because this is a little utility program, we don't bother to free
  buffers. The heap space will be released when the program exits. Actually we
  can't free the buffers made by getpwid() even if wanted to.

  1. We have to make the subu first so that we will have subu_uid and subu_gid
     to work with.

  2. Then we add user access via setfacl to masteru's home directory and to
     subu_land, so that we have permissions to make the home directory.

  3. Then as subu we create the home directory. .. I wonder where the system
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
#include <stdbool.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

#include <sys/stat.h>


#define DEBUG
typedef unsigned int uint;

#define ERR_ARG_CNT 1
#define ERR_SETUID_ROOT 2
#define ERR_BAD_MASTERU_HOME 3
#define ERR_NOT_EXIST_MASTERU_HOME 4
#define ERR_FAILED_MKDIR_SUBU 5
#define ERR_FAILED_RESTORATION 6

int main(int argc, char **argv, char **env){

  //--------------------------------------------------------------------------------
  // help message
  if( argc != 2 ){
    fprintf(stderr, "usage: %s subu", argv[0]);
    return ERR_ARG_CNT;
  }

  //--------------------------------------------------------------------------------
  // we must be invoked from a user account and be running as root
  uint uid = getuid();
  uint euid = geteuid();
  uint gid = getgid();
  uint egid = getegid();
  #ifdef DEBUG
    printf("uid %u, gid %u, euid %u\n", uid, gid, euid);
  #endif
  if( uid == 0 || euid != 0 ){
    fprintf(stderr, "this program must be run setuid root from a user account\n");
    return ERR_SETUID_ROOT;
  }

  //--------------------------------------------------------------------------------
  // who are these people anyway?
  char *subu_name = argv[1];
  struct passwd *passwd_record_pt = getpwuid(uid);
  char *masteru_name = passwd_record_pt->pw_name;
  // verify that subu_name is legal!  --> code goes here ...  

  //--------------------------------------------------------------------------------
  // build the subu_land path
  char *masteru_home_dir = passwd_record_pt->pw_dir;
  size_t masteru_home_dir_len = strlen(masteru_home_dir);
  if( masteru_home_dir_len == 0 || masteru_home_dir[0] == '(' ){
    fprintf(stderr,"strange, %s has no home directory\n", masteru_name);
    return ERR_BAD_MASTERU_HOME;
  }
  char *subu_land_extension = "/subu_land/";
  size_t subu_land_extension_len = strlen(subu_land_extension);
  size_t subu_name_len = strlen(subu_name); // we leave room in the buffer to latter add the subu_name
  char *subu_land = (char *)malloc( masteru_home_dir_len + subu_land_extension_len + subu_name_len + 1 );
  strcpy(subu_land, masteru_home_dir);
  strcpy(subu_land + masteru_home_dir_len, subu_land_extension);
  #ifdef DEBUG
  printf("The path to subu_land: %s\n", subu_land);
  #endif

  //--------------------------------------------------------------------------------
  // Just because masteru_home_dir is referenced in /etc/passwd does not mean it exists.
  // If it does, and the subu_land doesn't, then we make subu_land.
  struct stat st;
  if( stat(masteru_home_dir, &st) == -1) {
    fprintf(stderr, "Strange, masteru home does not exist, %s.", masteru_home_dir);
    return ERR_NOT_EXIST_MASTERU_HOME;
  }
  strcpy(subu_land + masteru_home_dir_len + subu_land_extension_len, subu_name);
  #ifdef DEBUG
  printf("A path to a subu: %s\n", subu_land);
  #endif

  //--------------------------------------------------------------------------------
  // we need to have a subu_uid and subu_gid to continue from here
  size_t subu_land_len = subu_land + masteru_home_dir_len + subu_land_extension_len + subu_name_len;
  
  

  // change to subu space
  if( seteuid(uid) == -1 || setegid(gid) == -1 ){ // we are root so this should never happen
    fprintf(stderr,"Strangely, root could not seteuid/setegid to %s\n", masteru_name);
    return ERR_FAILED_MKDIR_SUBU;
  }
  if( stat(subu_land, &st) == -1) { // then make the directory
    if( mkdir(subu_land, 0700) == -1 || stat(subu_land, &st) == -1 ){
      fprintf(stderr,"Failed to make subu directory %s\n", subu_land);
      return ERR_FAILED_MKDIR_SUBU;
    }
  }
  //change back to set the acls
  if( seteuid(euid) == -1 || setegid(egid) == -1 ){ 
    fprintf(stderr,"Could not restore privledges, having to bail.\n");
    return ERR_FAILED_RESTORATION;
  }
  return 0;
}
