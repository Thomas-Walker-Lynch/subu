/*
  sqllite3 is used to maintain the configuration file, which is currently compiled
  in as /etc/subu.db, (or just subu.db for testing).

  masteru is the user who ran this script. The masteru name comes from getuid
  and /etc/passwd. 

  subu is a subservient user.  The subuname is passed in as an argument.

  subu-mk-0 synthesizes a new user user name s<number>, calls useradd to creat
  the new uswer account, and enters the relationship between masteru, subu, and
  s<number> in the config file.  It is this relation in the config file that
  associates the subuname with the s<number> user.

  subu-rm-0 uses userdel to delete the related s<number> user account.  It
  then removes the relaton from the config file.

  subu-mk-0 and subu-rm-0 are setuid root scripts.  

*/
#include "subu.lib.h"

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

#if INTERFACE
#include <stdbool.h>
#include <errno.h>
#include <sqlite3.h>
#endif

//--------------------------------------------------------------------------------
// dispatched command errors  .. should add mkdir and rmdir ...
//
char *useradd_mess(int err){
  if(err <= 0) return NULL;
  char *mess;
  switch(err){
  case 1: mess = "can't update password file"; break;
  case 2: mess = "invalid command syntax"; break;
  case 3: mess = "invalid argument to option"; break;
  case 4: mess = "UID already in use (and no -o)"; break;
  case 5: mess = "undefined"; break;
  case 6: mess = "specified group doesn't exist"; break;
  case 7:
  case 8: mess = "undefined"; break;
  case 9: mess = "username already in use"; break;
  case 10: mess = "can't update group file:"; break;
  case 11: mess = "undefined"; break;
  case 12: mess = "can't create home directory"; break;
  case 13: mess = "undefined"; break;
  case 14: mess = "can't update SELinux user mapping"; break;
  default: mess = "undefined"; break;
  }
  return strdup(mess);
}
char *userdel_mess(int err){
  if(err <= 0) return NULL;
  char *mess;
  switch(err){
  case 1: mess = "can't update password file"; break;
  case 2: mess = "invalid command syntax"; break;
  case 3:
  case 4:
  case 5: mess = "undefined"; break;
  case 6: mess = "specified user doesn't exist"; break;
  case 7:
  case 8: 
  case 9: mess = "undefined"; break;
  case 10: mess = "can't update group file:"; break;
  case 11: mess = "undefined"; break;
  case 12: mess = "can't remove home directory"; break;
  default: mess = "undefined"; break;
  }
  return strdup(mess);
}


//--------------------------------------------------------------------------------
// an instance is subu_mk_0_ctx is returned by subu_mk_0
//
#if INTERFACE
#define SUBU_ERR_ARG_CNT 1
#define SUBU_ERR_SETUID_ROOT 2
#define SUBU_ERR_MALLOC 3
#define SUBU_ERR_MKDIR_SUBUHOME 4
#define SUBU_ERR_RMDIR_SUBUHOME 5
#define SUBU_ERR_SUBUNAME_MALFORMED 6
#define SUBU_ERR_MASTERU_HOMELESS 7
#define SUBU_ERR_CONFIG_FILE 8
#define SUBU_ERR_SUBUHOME_EXISTS 9
#define SUBU_ERR_BUG_SSS 10
#define SUBU_ERR_FAILED_USERADD 11
#define SUBU_ERR_FAILED_USERDEL 12
#define SUBU_ERR_CONFIG_SUBU_NOT_FOUND 13
#define SUBU_ERR_N 14
#endif

void subu_err(char *fname, int err, char *mess){
  if(!mess) mess = "";
  switch(err){
  case 0: return;
  case SUBU_ERR_ARG_CNT:
    if(mess[0])
      fprintf(stderr, "Incorrect number of arguments, %s", mess);
    else
      fprintf(stderr, "Incorrect number of arguments.", mess);
    break;
  case SUBU_ERR_SETUID_ROOT:
    fprintf(stderr, "This program must be run setuid root from a user account.");
    break;
  case SUBU_ERR_MALLOC:
    perror(fname);
    break;
  case SUBU_ERR_CONFIG_FILE:
    fprintf(stderr, "config file error: %s", Config_File); // Config_File is in common
    break;
  case SUBU_ERR_MASTERU_HOMELESS:
    fprintf(stderr,"Masteru, \"%s\", has no home directory", mess);
    break;
  case SUBU_ERR_SUBUNAME_MALFORMED:
    fprintf(stderr, "subuname, \"%s\" is not in [ _.-a-zA-Z0-9]*", mess);
    break;
  case SUBU_ERR_SUBUHOME_EXISTS:
    fprintf(stderr, "a file system object already exists at subuhome, \"%s\"\n", mess);
    break;
  case SUBU_ERR_MKDIR_SUBUHOME:
    fprintf(stderr, "masteru could not make subuhome, \"%s\"", mess);
    break;
  case SUBU_ERR_BUG_SSS:
    perror(fname);
  break;
  case SUBU_ERR_FAILED_USERADD:
    fprintf(stderr, "%s useradd failed\n", mess);
  break;
  default:
    fprintf(stderr, "unknown error code %d\n", err);
  }
  fputc('\n', stderr);
}


//--------------------------------------------------------------------------------
// a well formed subuname
// returns the length of the subuname, or -1
//
static int allowed_subuname(char **mess, char *subuname, size_t *subuname_len){
  char *ch = subuname;
  size_t i = 0;
  while(
        *ch
        &&
        ( *ch >= 'a' && *ch <= 'z'
          ||
          *ch >= 'A' && *ch <= 'Z'
          ||
          *ch >= '0' && *ch <= '9'
          ||
          *ch == '-'
          ||
          *ch == '_'
          ||
          *ch == '.'
          ||
          *ch == ' '
          )
        ){
    ch++;
    i++;
  }
  if( !*ch ){
    *subuname_len = i;
    return 0;
  }else{
    if(mess) *mess = strdup(subuname);
    return SUBU_ERR_SUBUNAME_MALFORMED;
  }
}

//--------------------------------------------------------------------------------
// dispatched functions
//
static int masteru_mkdir_subuhome(void *arg){
  char *subuhome = (char *) arg;
  if( mkdir( subuhome, Subuhome_Perms ) == -1 ){ // find subuhome perms in common
    perror("masteru_mkdir_subuhome");
    return SUBU_ERR_MKDIR_SUBUHOME;
  }
  return 0;
}
static int masteru_rmdir_subuhome(void *arg){
  char *subuhome = (char *) arg;
  if( rmdir( subuhome ) == -1 ){ // find subuhome perms in common
    perror("masteru_rmdir_subuhome");
    return SUBU_ERR_RMDIR_SUBUHOME;
  }
  return 0;
}

//--------------------------------------------------------------------------------
// build strings 
//
static int mk_subu_user(sqlite3 *db, char **mess, char **subu_username){
  char *ns=0; // 'ns'  Number as String
  if( subu_number_next( db, &ns, mess ) != SQLITE_OK ) return SUBU_ERR_CONFIG_FILE;
  size_t ns_len = strlen(ns);
  *subu_username = malloc(1 + ns_len + 1); // the first 1 is for the "s" prefix
  if( !*subu_username ) SUBU_ERR_MALLOC;
  strcpy(*subu_username, "s");
  strcpy(*subu_username + 1, ns);
  return 0;
}

// man page says that getpwuid strings may not be freed, I don't know how long until they
// are overwritten, so I just make my own copies that can be freed
static int mk_masteru_name(uid_t masteru_uid, char **masteru_name, char **masteru_home ){
  struct passwd *masteru_pw_record_pt = getpwuid(masteru_uid); // reading /etc/passwd
  *masteru_name = strdup(masteru_pw_record_pt->pw_name);
  *masteru_home = strdup(masteru_pw_record_pt->pw_dir);
  if( !masteru_home || !masteru_home[0] || (*masteru_home)[0] == '(' ) return SUBU_ERR_MASTERU_HOMELESS;
  return 0;
}

static int mk_subuland(char *masteru_home, char **subuland){
  size_t masteru_home_len = strlen(masteru_home);
  char *Subuland_Extension = "/subuland/";
  size_t Subuland_Extension_len = strlen(Subuland_Extension);
  *subuland = (char *)malloc( masteru_home_len + Subuland_Extension_len + 1 );
  if(!*subuland) SUBU_ERR_MALLOC; 
  strcpy(*subuland, masteru_home);
  strcpy(*subuland + masteru_home_len, Subuland_Extension);
  return 0;
}

static int mk_subuhome(char *subuland, char *subuname, char **subuhome){
  size_t subuland_len = strlen(subuland);
  size_t subuhome_len = subuland_len + strlen(subuname); // subuland has a trailing '/'
  *subuhome = (char *)malloc(subuhome_len + 1);
  if(!*subuhome) return SUBU_ERR_MALLOC;
  strcpy (*subuhome, subuland);
  strcpy (*subuhome + subuland_len, subuname);
  return 0;
}

#define RETURN(err)\
  {\
    free(subu_username);\
    free(masteru_name);\
    free(masteru_home);\
    free(subuland);\
    free(subuhome);\
    return err;\
  }

//===============================================================================
int subu_mk_0(char **mess, sqlite3 *db, char *subuname){

  int ret;
  if(mess)*mess = 0;

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Check that subuname is well formed and find its length\n");
  #endif
  size_t subuname_len;
  ret = allowed_subuname(mess, subuname, &subuname_len);
  if(ret) return ret;

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Check that we are running from a user and are setuid root.\n");
  #endif
  uid_t masteru_uid;
  gid_t masteru_gid;
  uid_t set_euid;
  gid_t set_egid;
  {
    masteru_uid = getuid();
    masteru_gid = getgid();
    set_euid = geteuid();
    set_egid = getegid();
    #ifdef DEBUG
    dbprintf("masteru_uid %u, masteru_gid %u, set_euid %u set_egid %u\n", masteru_uid, masteru_gid, set_euid, set_egid);
    #endif
    if( masteru_uid == 0 || set_euid != 0 ) return SUBU_ERR_SETUID_ROOT;
  }

  //--------------------------------------------------------------------------------
  // various strings that we will need
  char *subu_username = 0;
  char *masteru_name = 0;
  char *masteru_home = 0;
  char *subuland = 0;
  char *subuhome = 0; // the name of the directory to put in subuland, not subu_user home dir
  ret =
    mk_subu_user(db, mess,  &subu_username)
    ||
    mk_masteru_name(masteru_uid, &masteru_name, &masteru_home)
    ||
    mk_subuland(masteru_home, &subuland)
    ||
    mk_subuhome(subuland, subuname, &subuhome)
    ;
  if(ret) RETURN(ret);

  //--------------------------------------------------------------------------------
  // By having masteru create the subuhome, we know that masteru has rights to 
  // to access this directory. This will be the mount point for bindfs
  {
    #ifdef DEBUG
    dbprintf("as masteru, making the directory \"%s\"\n", subuhome);
    #endif
    struct stat st;
    if( stat(subuhome, &st) != -1 ){
      if(mess)*mess = strdup(subuhome);
      RETURN(SUBU_ERR_SUBUHOME_EXISTS);
    }
    int dispatch_err = dispatch_f_euid_egid
      (
       "masteru_mkdir_subuhome", 
       masteru_mkdir_subuhome, 
       (void *)subuhome,
       masteru_uid, 
       masteru_gid
       );
    if( dispatch_err <= ERR_DISPATCH || dispatch_err == SUBU_ERR_MKDIR_SUBUHOME ){
      #ifdef DEBUG
      dispatch_f_mess("dispatch_f_euid_egid", dispatch_err, "masteru_mkdir_subuhome");
      #endif
      if(mess)*mess = strdup(subuhome);
      RETURN(SUBU_ERR_MKDIR_SUBUHOME);
    }
  }
  #ifdef DEBUG
  dbprintf("masteru made directory \"%s\"\n", subuhome);
  #endif

  //--------------------------------------------------------------------------------
  //  Make the subservient user account, i.e. the subu
  {
    #ifdef DEBUG
      dbprintf("making subu \"%s\" as user \"%s\"\n", subuname, subu_username);
    #endif
    #if BUG_SSS_CACHE_RUID
      #ifdef DEBUG
        dbprintf("setting inherited real uid to 0 to accomodate SSS_CACHE UID BUG\n");
      #endif
      if( setuid(0) == -1 ) RETURN(SUBU_ERR_BUG_SSS);
    #endif
    char *command = "/usr/sbin/useradd";
    char *argv[3];
    argv[0] = command;
    argv[1] = subu_username;
    argv[2] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    int dispatch_err = dispatch_exec(argv, envp);
    if( dispatch_err != 0 ){
      #ifdef DEBUG 
      dispatch_f_mess("dispatch_exec", dispatch_err, command);
      #endif
      // go back and remove the directory we made in subuland
      int dispatch_err_rmdir = dispatch_f_euid_egid
        (
         "masteru_rmdir_subuhome", 
         masteru_rmdir_subuhome, 
         (void *)subuhome,
         masteru_uid, 
         masteru_gid
         );
      #ifdef DEBUG
      dispatch_f_mess("dispatch_f_euid_egid", dispatch_err_rmdir, "masteru_rmdir_subuhome");
      #endif
      if(mess)*mess = useradd_mess(dispatch_err);
      RETURN(SUBU_ERR_FAILED_USERADD);
    }
    #ifdef DEBUG
    dbprintf("added user \"%s\"\n", subu_username);
    #endif
  }  
  
  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("setting the masteru_name, subuname, subu_username relation\n");
  #endif
  {
    int ret = subu_Masteru_Subu_put(db, masteru_name, subuname, subu_username);
    if( ret != SQLITE_DONE ){
      if(mess)*mess = strdup("insert of masteru subu relation failed");
      RETURN(SUBU_ERR_CONFIG_FILE);
    }
  }
  #ifdef DEBUG
  dbprintf("finished subu-mk-0(%s)\n", subuname);
  #endif
  RETURN(0);
}

//================================================================================
int subu_rm_0(char **mess, sqlite3 *db, char *subuname){

  int ret;
  if(mess)*mess = 0;

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Check that subuname is well formed and find its length\n");
  #endif
  size_t subuname_len;
  ret = allowed_subuname(mess, subuname, &subuname_len);
  if(ret) return ret;
  
  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Check that we are running from a user and are setuid root.\n");
  #endif
  uid_t masteru_uid;
  gid_t masteru_gid;
  uid_t set_euid;
  gid_t set_egid;
  {
    masteru_uid = getuid();
    masteru_gid = getgid();
    set_euid = geteuid();
    set_egid = getegid();
    #ifdef DEBUG
    dbprintf("masteru_uid %u, masteru_gid %u, set_euid %u set_egid %u\n", masteru_uid, masteru_gid, set_euid, set_egid);
    #endif
    if( masteru_uid == 0 || set_euid != 0 ) return SUBU_ERR_SETUID_ROOT;
  }

  //--------------------------------------------------------------------------------
  // various strings that we will need
  char *subu_username = 0;
  char *masteru_name = 0;
  char *masteru_home = 0;
  char *subuland = 0;
  char *subuhome = 0; // the name of the directory to put in subuland, not subu_user home dir
  ret =
    mk_masteru_name(masteru_uid, &masteru_name, &masteru_home)
    ||
    mk_subuland(masteru_home, &subuland)
    ||
    mk_subuhome(subuland, subuname, &subuhome)
    ;
  if(ret) RETURN(ret);

  #ifdef DEBUG
  dbprintf("looking up subu_username given masteru_name/subuname\n");
  #endif
  {
    int sgret = subu_Masteru_Subu_get(db, masteru_name, subuname, &subu_username);
    if( sgret != SQLITE_DONE ){
      if(mess) *mess = strdup("subu requested for removal not found under this masteru in config file");
      ret = SUBU_ERR_CONFIG_SUBU_NOT_FOUND;
      RETURN(ret);
    }
    #ifdef DEBUG
    printf("subu_username: %s\n", subu_username);  
    #endif
  }

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("remove the masteru_name, subuname, subu_username relation\n");
  #endif
  {
    int ret = subu_Masteru_Subu_rm(db, masteru_name, subuname, subu_username);
    if( ret != SQLITE_DONE ){
      if(mess)*mess = strdup("removal of masteru subu relation failed");
      RETURN(SUBU_ERR_CONFIG_FILE);
    }
  }

  //--------------------------------------------------------------------------------
  // Only masteru can remove directories from masteru/subuland, so we switch to 
  // masteru's uid for performing the rmdir.
  //
  {
    #ifdef DEBUG
    dbprintf("as masteru, removing the directory \"%s\"\n", subuhome);
    #endif
    int dispatch_err = dispatch_f_euid_egid
      (
       "masteru_rmdir_subuhome", 
       masteru_rmdir_subuhome, 
       (void *)subuhome,
       masteru_uid, 
       masteru_gid
       );
    if( dispatch_err <= ERR_DISPATCH || dispatch_err == SUBU_ERR_RMDIR_SUBUHOME ){
      #ifdef DEBUG
      dispatch_f_mess("dispatch_f_euid_egid", dispatch_err, "masteru_rmdir_subuhome");
      #endif
      if(mess)*mess = strdup(subuhome);
      RETURN(SUBU_ERR_RMDIR_SUBUHOME);
    }
  }

  //--------------------------------------------------------------------------------
  //  Delete the subservient user account
  {
    #ifdef DEBUG
    dbprintf("deleting user \"%s\"\n", subu_username);
    #endif
    #if BUG_SSS_CACHE_RUID
      #ifdef DEBUG
        dbprintf("setting inherited real uid to 0 to accomodate SSS_CACHE UID BUG\n");
      #endif
      if( setuid(0) == -1 ){
        ret = SUBU_ERR_BUG_SSS;
        RETURN(ret);
      }
    #endif
    char *command = "/usr/sbin/userdel";
    char *argv[3];
    argv[0] = command;
    argv[1] = subu_username;
    argv[2] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    int dispatch_err = dispatch_exec(argv, envp);
    if( dispatch_err != 0 ){
      #ifdef DEBUG 
      dispatch_f_mess("dispatch_exec", dispatch_err, command);
      #endif
      if(mess)*mess = userdel_mess(dispatch_err);
      RETURN(SUBU_ERR_FAILED_USERDEL);
    }
    #ifdef DEBUG
    dbprintf("deleted user \"%s\"\n", subu_username);
    #endif
  }  

  #ifdef DEBUG
  dbprintf("finished subu-rm-0(%s)\n", subuname);
  #endif
  RETURN(0);
}

