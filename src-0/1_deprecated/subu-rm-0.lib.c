/*
  subu-rm-0 subuname

  1. get our uid and lookup masteru_name in /etc/passwd
  2. lookup masteru_name/subuname in config file, which gives us subu_username
  3. unmount subuland/subuname
  4. userdel subu_username
  5. rmdir subuland/subuname

  Note, as per the man page, we are not allowed to free the memory allocated by getpwid().

*/
#include "subu-mk-0.lib.h"

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
// an instance is subu_rm_0_ctx is returned by subu_rm_0
//
#if INTERFACE
#define ERR_SUBU_RM_0_MKDIR_SUBUHOME 1
#define ERR_SUBU_RM_0_RMDIR_SUBUHOME 2
#define ERR_SUBU_RM_0_SUBUNAME_MALFORMED 3
#define ERR_SUBU_RM_0_SETUID_ROOT 4
#define ERR_SUBU_RM_0_MASTERU_HOMELESS 5
#define ERR_SUBU_RM_0_MALLOC 6
#define ERR_SUBU_RM_0_CONFIG_FILE 7
#define ERR_SUBU_RM_0_SUBUHOME_EXISTS 8
#define ERR_SUBU_RM_0_BUG_SSS 9
#define ERR_SUBU_RM_0_FAILED_USERADD 10

struct subu_rm_0_ctx{
  char *name;
  char *subuland;
  char *subuhome;
  char *subu_username;
  bool free_aux;
  char *aux;
  uint err;
};
#endif
struct subu_rm_0_ctx *subu_rm_0_ctx_mk(){
  struct subu_rm_0_ctx *ctxp = malloc(sizeof(struct subu_rm_0_ctx));
  ctxp->name = "subu_rm_0";
  ctxp->subuland = 0;
  ctxp->subuhome = 0;
  ctxp->subu_username = 0;
  ctxp->free_aux = false;
  ctxp->aux = 0;
}
void subu_rm_0_ctx_free(struct subu_rm_0_ctx *ctxp){
  free(ctxp->subuland);
  free(ctxp->subuhome);
  free(ctxp->subu_username);
  if(ctxp->free_aux) free(ctxp->aux);
  free(ctxp);
}
// must be called before any system calls, otherwise perror() will be messed up
void subu_rm_0_mess(struct subu_rm_0_ctx *ctxp){
  switch(ctxp->err){
  case 0: return;
  case ERR_SUBU_RM_0_MKDIR_SUBUHOME:
    fprintf(stderr, "masteru could not make subuhome, \"%s\"", ctxp->subuhome);
    break;
  case ERR_SUBU_RM_0_SUBUNAME_MALFORMED:
    fprintf(stderr, "subuname, \"%s\" is not in [ _.-a-zA-Z0-9]*", ctxp->aux);
    break;
  case ERR_SUBU_RM_0_SETUID_ROOT:
    fprintf(stderr, "This program must be run setuid root from a user account.");
    break;
  case ERR_SUBU_RM_0_MASTERU_HOMELESS:
    fprintf(stderr,"Masteru, \"%s\", has no home directory", ctxp->aux);
    break;
  case ERR_SUBU_RM_0_MALLOC:
    perror(ctxp->name);
  break;
  case ERR_SUBU_RM_0_CONFIG_FILE:
    fprintf(stderr, "config file error: %s", ctxp->aux);
  break;
  case ERR_SUBU_RM_0_SUBUHOME_EXISTS:
    fprintf(stderr, "a file system object already exists at subuhome, \"%s\"\n", ctxp->subuhome);
  break;
  case ERR_SUBU_RM_0_BUG_SSS:
    perror(ctxp->name);
  break;
  case ERR_SUBU_RM_0_FAILED_USERADD:
    fprintf(stderr, "%u useradd failed\n", ctxp->subu_username);
  break;
  default:
    fprintf(stderr, "unknown error code %d\n", ctxp->err);
  }
  fputc('\n', stderr);
}

//--------------------------------------------------------------------------------
// dispatched functions
//
// the making of subuhome is dispatched to its own process so as to give it its own uid/gid
static int masteru_mkdir_subuhome(void *arg){
  char *subuhome = (char *) arg;
  if( mkdir( subuhome, subuhome_perms ) == -1 ){ // find subuhome perms in common
    perror("masteru_mkdir_subuhome");
    return ERR_SUBU_RM_0_MKDIR_SUBUHOME;
  }
  return 0;
}
static int masteru_rmdir_subuhome(void *arg){
  char *subuhome = (char *) arg;
  if( rmdir( subuhome ) == -1 ){ // find subuhome perms in common
    perror("masteru_rmdir_subuhome");
    return ERR_SUBU_RM_0_RMDIR_SUBUHOME;
  }
  return 0;
}

//--------------------------------------------------------------------------------
//  the public call point
struct subu_rm_0_ctx *subu_rm_0(sqlite3 *db, char *subuname){

  struct subu_rm_0_ctx *ctxp = subu_rm_0_ctx_mk();

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Checking that subuname is well formed and finding its length\n");
  #endif
  size_t subuname_len;
  {
    int ret = allowed_subuname(subuname, &subuname_len);
    if( ret == -1 ){
      ctxp->err = ERR_SUBU_RM_0_SUBUNAME_MALFORMED;
      ctxp->aux = subuname;
      return ctxp;
    }}
  
  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Checking that we are running from a user and are setuid root.\n");
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
    if( masteru_uid == 0 || set_euid != 0 ){
      ctxp->err = ERR_SUBU_RM_0_SETUID_ROOT;
      return ctxp;
    }
  }

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("strings masteru_name and masteru_home\n");
  #endif

  char *masteru_name;
  size_t masteru_name_len;
  char *masteru_home;
  size_t masteru_home_len;
  size_t subuland_len;
  {
    struct passwd *masteru_pw_record_pt = getpwuid(masteru_uid); // reading /etc/passwd
    masteru_name = masteru_pw_record_pt->pw_name;
    masteru_name_len = strlen(masteru_name);
    #ifdef DEBUG
    dbprintf("masteru_name \"%s\" %zu\n", masteru_name, masteru_name_len);
    #endif
    masteru_home = masteru_pw_record_pt->pw_dir;
    masteru_home_len = strlen(masteru_home);
    #ifdef DEBUG
    dbprintf("masteru_home \"%s\" %zu\n", masteru_home, masteru_home_len);
    #endif
    masteru_home_len = strlen(masteru_home);
    if( masteru_home_len == 0 || masteru_home[0] == '(' ){
      ctxp->err = ERR_SUBU_RM_0_MASTERU_HOMELESS;
      ctxp->aux = masteru_name;  // we can not free a passwd struct, or its fields.  I assume then it isn't re-entrant safe.
      return ctxp;
    }
    // char *subuland_extension = "/subuland/"; // moved to common.lib.c
    size_t subuland_extension_len = strlen(subuland_extension);
    ctxp->subuland = (char *)malloc( masteru_home_len + subuland_extension_len + 1 );
    if(!ctxp->subuland){
      ctxp->err = ERR_SUBU_RM_0_MALLOC;
      return ctxp;
    }
    strcpy(ctxp->subuland, masteru_home);
    strcpy(ctxp->subuland + masteru_home_len, subuland_extension);
    subuland_len = masteru_home_len + subuland_extension_len;
    #ifdef DEBUG
    dbprintf("subuland \"%s\" %zu\n", ctxp->subuland, subuland_len);
    #endif
  }

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("lookup subu_username from masteru_name/subuname in config file\n");
  #endif
  char *subu_username; // this is part of ctx and must be freed
  {
    int ret = subu_get_masteru_subu(db, masteru_name, subuname, &subu_username);
    if( ret != SQLITE_DONE ){
      printf("get failed\n");
      return 2;
    }
    #ifdef DEBUG
    printf("subu_username: %s\n", subu_username);  
    #endif
    
  }

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("strings subu_username and subuhome\n");
  #endif
  size_t subu_username_len;
  size_t subuhome_len;
  {
    char *ns=0; // 'ns'  Number as String
    char *mess=0;
    if( subu_number_get( db, &ns, &mess ) != SQLITE_OK ){
      ctxp->err = ERR_SUBU_RM_0_CONFIG_FILE;
      ctxp->aux = mess;
      ctxp->free_aux = true;
      return ctxp;
    }
    size_t ns_len = strlen(ns);
    ctxp->subu_username = malloc(1 + ns_len + 1);
    if( !ctxp->subu_username ){
      ctxp->err = ERR_SUBU_RM_0_MALLOC;
      return ctxp;
    }
    strcpy(ctxp->subu_username, "s");
    strcpy(ctxp->subu_username + 1, ns);
    subu_username_len = ns_len + 1;
    #ifdef DEBUG
    dbprintf("subu_username \"%s\" %zu\n", ctxp->subu_username, subu_username_len);
    #endif

    subuhome_len = subuland_len + subuname_len; 
    ctxp->subuhome = (char *)malloc(subuhome_len + 1);
    if( !ctxp->subuhome ){
      ctxp->err = ERR_SUBU_RM_0_MALLOC;
      return ctxp;
    }
    strcpy (ctxp->subuhome, ctxp->subuland);
    strcpy (ctxp->subuhome + subuland_len, subuname);
    #ifdef DEBUG
    dbprintf("subuhome \"%s\" %zu\n", ctxp->subuhome, subuhome_len);
    #endif
  }

  //--------------------------------------------------------------------------------
  // By having masteru create the subuhome, we know that masteru has rights to 
  // to access this directory. This will be the mount point for bindfs
  {
    #ifdef DEBUG
    dbprintf("as masteru, making the directory \"%s\"\n", ctxp->subuhome);
    #endif
    struct stat st;
    if( stat(ctxp->subuhome, &st) != -1 ){
      ctxp->err = ERR_SUBU_RM_0_SUBUHOME_EXISTS;
      return ctxp;
    }
    dispatch_ctx *dfr = dispatch_f_euid_egid
      (
       "masteru_mkdir_subuhome", 
       masteru_mkdir_subuhome, 
       (void *)ctxp->subuhome,
       masteru_uid, 
       masteru_gid
       );
    if( dfr->err <= ERR_DISPATCH || dfr->err == ERR_SUBU_RM_0_MKDIR_SUBUHOME ){
      #ifdef DEBUG
      if( dfr->err == ERR_SUBU_RM_0_MKDIR_SUBUHOME )
        perror("mkdir");
      else
        dispatch_f_mess(dfr);
      #endif
      ctxp->err = ERR_SUBU_RM_0_MKDIR_SUBUHOME;
      return ctxp;
    }
  }
  #ifdef DEBUG
  dbprintf("masteru made directory \"%s\"\n", ctxp->subuhome);
  #endif

  //--------------------------------------------------------------------------------
  //  Make the subservient user account, i.e. the subu
  {
    #ifdef DEBUG
      dbprintf("making subu \"%s\" as user \"%s\"\n", subuname, ctxp->subu_username);
    #endif
    #if BUG_SSS_CACHE_RUID
      #ifdef DEBUG
        dbprintf("setting inherited real uid to 0 to accomodate SSS_CACHE UID BUG\n");
      #endif
      if( setuid(0) == -1 ){
        ctxp->err = ERR_SUBU_RM_0_BUG_SSS;
        return ctxp;
      }
    #endif
    char *command = "/usr/sbin/useradd";
    char *argv[3];
    argv[0] = command;
    argv[1] = ctxp->subu_username;
    argv[2] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    dispatch_ctx *dfr = dispatch_exec(argv, envp);
    if( dfr->err != 0 ){
      #ifdef DEBUG 
      if( dfr->err <= ERR_DISPATCH )
        dispatch_f_mess(dfr);
      else
        perror("useradd");
      #endif
      // go back and remove the directory we made in subuland
      dispatch_ctx *dfr = dispatch_f_euid_egid
        (
         "masteru_rmdir_subuhome", 
         masteru_rmdir_subuhome, 
         (void *)ctxp->subuhome,
         masteru_uid, 
         masteru_gid
         );
      #ifdef DEBUG
        if( dfr->err <= ERR_DISPATCH || dfr->err == ERR_SUBU_RM_0_RMDIR_SUBUHOME )
          if( dfr->err == ERR_SUBU_RM_0_RMDIR_SUBUHOME )
            perror("rmdir");
          else
            dispatch_f_mess(dfr);
      #endif
      ctxp->err = ERR_SUBU_RM_0_FAILED_USERADD;
      return ctxp;
    }
    #ifdef DEBUG
    dbprintf("added user \"%s\"\n", ctxp->subu_username);
    #endif
  }  
  
  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("setting the masteru_name, subuname, subu_username relation\n");
  #endif
  {
    int ret = subu_put_masteru_subu(db, masteru_name, subuname, ctxp->subu_username);
    if( ret != SQLITE_DONE ){
      ctxp->err = ERR_SUBU_RM_0_CONFIG_FILE;
      ctxp->aux = "insert of masteru subu relation failed";
      return ctxp;
    }
  }

  #ifdef DEBUG
  dbprintf("finished subu-mk-0(%s)\n", subuname);
  #endif
  ctxp->err = 0;
  return ctxp;
}