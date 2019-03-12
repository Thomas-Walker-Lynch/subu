/*
  sqllite3 is used to maintain the db file, which is currently compiled
  in as /etc/subu.db, (or just subu.db for testing).

  masteru is the user who ran this script. The masteru name comes from getuid
  and /etc/passwd. 

  subu is a subservient user.  The subuname is passed in as an argument.

  subu-mk-0 synthesizes a new user user name s<number>, calls useradd to creat
  the new uswer account, and enters the relationship between masteru, subu, and
  s<number> in the db file.  It is this relation in the db file that
  associates the subuname with the s<number> user.

  subu-rm-0 uses userdel to delete the related s<number> user account.  It
  then removes the relaton from the db file.

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
//
#if INTERFACE
#define SUBU_ERR_ARG_CNT 1
#define SUBU_ERR_SETUID_ROOT 2
#define SUBU_ERR_MALLOC 3
#define SUBU_ERR_MKDIR_SUBUHOME 4
#define SUBU_ERR_RMDIR_SUBUHOME 5
#define SUBU_ERR_SUBUNAME_MALFORMED 6
#define SUBU_ERR_MASTERU_HOMELESS 7
#define SUBU_ERR_DB_FILE 8
#define SUBU_ERR_SUBUHOME_EXISTS 9
#define SUBU_ERR_BUG_SSS 10
#define SUBU_ERR_FAILED_USERADD 11
#define SUBU_ERR_FAILED_USERDEL 12
#define SUBU_ERR_SUBU_NOT_FOUND 13
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
  case SUBU_ERR_DB_FILE:
    fprintf(stderr, "db file error: %s", DB_File); // DB_File is in common
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
static int mk_subu_user(char **mess, sqlite3 *db, char *masteru_name, int n, char **subu_username){
  size_t len = 0;
  FILE* name_stream = open_memstream(subu_username, &len);
  fprintf(name_stream, "s%d", n);
  fclose(name_stream);
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

  int rc;
  if(mess)*mess = 0;

  //--------------------------------------------------------------------------------
  size_t subuname_len;
  rc = allowed_subuname(mess, subuname, &subuname_len);
  if(rc) return rc;
  #ifdef DEBUG
  dbprintf("subuname is well formed\n");
  #endif

  //--------------------------------------------------------------------------------
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
  // lookup the masteru name
  char *masteru_name = 0;
  char *masteru_home = 0;
  rc = mk_masteru_name(masteru_uid, &masteru_name, &masteru_home);
  if(rc) return rc;
  #ifdef DEBUG
  dbprintf("masteru_name: \"%s\"\n", masteru_name);
  #endif

  db_begin(db);
  int n;
  rc = subudb_number_get(db, masteru_name, &n);
  if( rc == SQLITE_OK ){
    n++;
    rc = subudb_number_set(db, masteru_name, n);
    if( rc != SQLITE_OK ){
      db_rollback(db);
      return SUBU_ERR_DB_FILE;
    }
  }else{ // perhaps this masteru's first subu, so we will try init
    n = First_Max_Subunumber;
    rc = subudb_number_init(db, masteru_name, n);
    if( rc != SQLITE_OK ){
      db_rollback(db);
      return SUBU_ERR_DB_FILE;
    }
  }
  db_commit(db);
  #ifdef DEBUG
  dbprintf("masteru max subunumber: %d\n", n);
  #endif

  //--------------------------------------------------------------------------------
  // subu details
  char *subu_username = 0;
  char *subuland = 0;
  char *subuhome = 0; // the name of the directory to put in subuland, not subu_user home dir
  rc =
    mk_subu_user(mess, db, masteru_name, n, &subu_username)
    ||
    mk_subuland(masteru_home, &subuland)
    ||
    mk_subuhome(subuland, subuname, &subuhome)
    ;
  if(rc) RETURN(rc);
  #ifdef DEBUG
  dbprintf("subu_username, subuland, subuhome: \"%s\"\"%s\"\"%s\"\n", subu_username, subuland, subuhome);
  #endif

  //--------------------------------------------------------------------------------
  // By having masteru create the subuhome, we know that masteru has rights to 
  // to access this directory. This will be the mount point for bindfs
  {
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
  dbprintf("made directory \"%s\"\n", subuhome);
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
    int rc = subudb_Masteru_Subu_put(db, masteru_name, subuname, subu_username);
    if( rc != SQLITE_DONE ){
      if(mess)*mess = strdup("insert of masteru subu relation failed");
      RETURN(SUBU_ERR_DB_FILE);
    }
  }
  #ifdef DEBUG
  dbprintf("finished subu-mk-0(%s)\n", subuname);
  #endif
  RETURN(0);
}

//================================================================================
int subu_rm_0(char **mess, sqlite3 *db, char *subuname){

  int rc;
  if(mess)*mess = 0;

  //--------------------------------------------------------------------------------
  size_t subuname_len;
  rc = allowed_subuname(mess, subuname, &subuname_len);
  if(rc) return rc;
  #ifdef DEBUG
  dbprintf("subuname is well formed\n");
  #endif
  
  //--------------------------------------------------------------------------------
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
  char *masteru_name = 0;
  char *masteru_home = 0;
  char *subuland = 0;
  char *subuhome = 0; // the name of the directory to put in subuland, not subu_user home dir
  rc =
    mk_masteru_name(masteru_uid, &masteru_name, &masteru_home)
    ||
    mk_subuland(masteru_home, &subuland)
    ||
    mk_subuhome(subuland, subuname, &subuhome)
    ;
  if(rc) RETURN(rc);
  #ifdef DEBUG
  dbprintf("masteru_home, subuhome: \"%s\", \"%s\"\n", masteru_home, subuhome);
  #endif

  //--------------------------------------------------------------------------------
  // removal from db

  db_begin(db);

  char *subu_username = 0;
  rc = subudb_Masteru_Subu_get_subu_username(db, masteru_name, subuname, &subu_username);
  if( rc != SQLITE_OK ){
    if(mess) *mess = strdup("subu requested for removal not found under this masteru in db file");
    rc = SUBU_ERR_SUBU_NOT_FOUND;
    db_rollback();
    RETURN(rc);
  }
  #ifdef DEBUG
  printf("subu_username: \"%s\"\n", subu_username);  
  #endif

  rc = subudb_Masteru_Subu_rm(db, masteru_name, subuname, subu_username);
  if( rc != SQLITE_OK ){
    if(mess)*mess = strdup("removal of masteru subu relation failed");
    db_rollback();
    RETURN(SUBU_ERR_DB_FILE);
  }
  #ifdef DEBUG
  dbprintf("removed the masteru_name, subuname, subu_username relation\n");
  #endif
  
  rc = db_commit(db);
  if( rc != SQLITE_OK ){
    if(mess)*mess = strdup("removal of masteru subu relation in unknown state, exiting");
    RETURN(SUBU_ERR_DB_FILE);
  }
  
  // even after removing the last masteru subu relation, we still do not remove
  // the max subu count. Hence, a masteru will keep such for a life time.


  //--------------------------------------------------------------------------------
  // Only masteru can remove directories from masteru/subuland, so we switch to 
  // masteru's uid to perform the rmdir.
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
        rc = SUBU_ERR_BUG_SSS;
        RETURN(rc);
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

#if 0
//================================================================================
// identifies masteru, the bindfs maps each subu_user's home to its mount point
// in subuland.
int subu_bind(char **mess, char *subu_user_home, char *subuhome){
    char *command = "/usr/bin/bindfs";
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
int subu_bind_all(char **mess, sqlite3 *db, char *subuname){

  int rc;
  if(mess)*mess = 0;

  //--------------------------------------------------------------------------------
  size_t subuname_len;
  rc = allowed_subuname(mess, subuname, &subuname_len);
  if(rc) return rc;
  #ifdef DEBUG
  dbprintf("subuname is well formed\n");
  #endif
  
  //--------------------------------------------------------------------------------
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
  char *masteru_name = 0;
  char *masteru_home = 0;
  char *subuland = 0;
  char *subuhome = 0; // the name of the directory to put in subuland, not subu_user home dir
  rc =
    mk_masteru_name(masteru_uid, &masteru_name, &masteru_home)
    ||
    mk_subuland(masteru_home, &subuland)
    ||
    mk_subuhome(subuland, subuname, &subuhome)
    ;
  if(rc) RETURN(rc);
  #ifdef DEBUG
  dbprintf("masteru_home, subuhome: \"%s\", \"%s\"\n", masteru_home, subuhome);
  #endif

  //--------------------------------------------------------------------------------
  // removal from db

  db_begin(db);

  char *subu_username = 0;
  rc = subudb_Masteru_Subu_get_subu_username(db, masteru_name, subuname, &subu_username);
  if( rc != SQLITE_OK ){
    if(mess) *mess = strdup("subu requested for removal not found under this masteru in db file");
    rc = SUBU_ERR_SUBU_NOT_FOUND;
    db_rollback();
    RETURN(rc);
  }
  #ifdef DEBUG
  printf("subu_username: \"%s\"\n", subu_username);  
  #endif

  rc = subudb_Masteru_Subu_rm(db, masteru_name, subuname, subu_username);
  if( rc != SQLITE_OK ){
    if(mess)*mess = strdup("removal of masteru subu relation failed");
    db_rollback();
    RETURN(SUBU_ERR_DB_FILE);
  }
  #ifdef DEBUG
  dbprintf("removed the masteru_name, subuname, subu_username relation\n");
  #endif
  
  rc = db_commit(db);
  if( rc != SQLITE_OK ){
    if(mess)*mess = strdup("removal of masteru subu relation in unknown state, exiting");
    RETURN(SUBU_ERR_DB_FILE);
  }
  
  // even after removing the last masteru subu relation, we still do not remove
  // the max subu count. Hence, a masteru will keep such for a life time.


  //--------------------------------------------------------------------------------
  // Only masteru can remove directories from masteru/subuland, so we switch to 
  // masteru's uid to perform the rmdir.
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
        rc = SUBU_ERR_BUG_SSS;
        RETURN(rc);
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

#endif
