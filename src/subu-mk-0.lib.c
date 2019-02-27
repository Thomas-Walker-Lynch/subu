/*
  Makes a new subu user. This routine creates the bindfs mount point under
  subuland, creates the _s<number>  user, and adds the master / subu relation
  to configuration file.

  masteru is the user who ran this script. subu is a subservient user.  subuname
  is passed as an argument, we get the materu name from getuid and /etc/passwd.

  subu-mk-0 is a setuid root script. 

  sqllite3 is used to maintain the configuration file, which is currently /etc/subu.db

  useradd is called to make the _s<number> user

  Note, as per the man page, we are not allowed to free the memory allocated by getpwid().


*/
#include "subu-mk-0.lib.h"

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


/*
  Fedora 29's sss_cache is checking the inherited uid instead of the effective
  uid, so setuid root scripts will fail when calling sss_cache. Fedora 29's
  'useradd'  calls sss_cache.
 
*/
#define BUG_SSS_CACHE_RUID 1
  
// a well formed subuname
// wonder if it makes sense to add a length limit ...
// returns the length of the subuname, or -1
int allowed_subuname(char *subuname, size_t &subuname_len){
  char *ch = subuname;
  uint i = 0;
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
    subuname_len = i;
    return 0;
  }else
    return -1;
}

struct subu_mk_0_ctx{
  char *name;
  char *subuland;
  char *subuhome;
  char *subu_username;
  char *aux;
  uint err;
};
ctxp *subu_mk_0_ctx_mk(){
  ctxp = malloc(sizeof(subu_mk_0_ctx));
  ctxp->name = "subu_mk_0";
  ctxp->subuland = 0;
  ctxp->subuhome = 0;
  ctxp->subu_username = 0;
  ctxp->free_aux = false;
  ctxp->aux = 0;
}
void subu_mk_0_ctx_free(subu_mk_0_ctx ctxp){
  free(ctxp->subuland);
  free(ctxp->subuhome);
  free(ctxp->subu_username);
  if(ctxp->free_aux) free(ctxp->aux);
  free ctxp;
}

#if INTERFACE
#include <sqlite3.h>
#define ERR_SUBU_MK_0_MKDIR_SUBUHOME 1
#define ERR_SUBU_MK_0_SUBUNAME_MALFORMED 2
#define ERR_SUBU_MK_0_SETUID_ROOT 3
#define ERR_SUBU_MK_0_MASTERU_HOMELESS 4
#define ERR_SUBU_MK_0_MALLOC 5
#define ERR_SUBU_MK_0_CONFIG_FILE 6
#define ERR_SUBU_MK_0_SUBUHOME_EXISTS 7
#define ERR_SUBU_MK_0_BUG_SSS 8
#define ERR_SUBU_MK_0_FAILED_USERADD 9
#define 
#endif
// must be called before any system calls, or perror() will be messed up
void subu_mk_0_mess(struct subu_mk_0_ctx *ctxp){
  switch(ctxp->err){
  case 0: return;
  case ERR_SUBU_MK_0_MKDIR_SUBUHOME:
    fprintf(stderr, "masteru could not make subuhome, \"%s\"", ctxp->subuhome);
    break;
  case ERR_SUBU_MK_0_SUBUNAME_MALFORMED:
    fprintf(stderr, "subuname, \"%s\" is not in [ _.\-a-zA-Z0-9]*", ctxp->aux);
    break;
  case ERR_SUBU_MK_0_SETUID_ROOT:
    fprintf(stderr, "This program must be run setuid root from a user account.");
    break;
  case ERR_SUBU_MK_0_MASTERU_HOMELESS:
    char *masteru_name = aux; // this should not be freed
    fprintf(stderr,"Masteru, \"%s\", has no home directory", masteru_name);
    break;
  case ERR_SUBU_MK_0_MALLOC 
    perror(ctxp->name);
    break;
  case ERR_SUBU_MK_0_CONFIG_FILE
    fprintf(stderr, "config file error: %s", ctxp->aux);
    break;
  case ERR_SUBU_MK_0_SUBUHOME_EXISTS
    fprintf(stderr, "a file system object already exists at subuhome, \"%s\"\n", ctxp->subuhome);
    break;
  case ERR_SUBU_MK_0_BUG_SSS 
    perror(ctxp->name);
    break;
  case ERR_SUBU_MK_0_FAILED_USERADD 
    fprintf(stderr, "%u useradd failed\n", ctxp->subu_username);
    break;
  default:
  }
  fputc('\n', stderr);
}

// will be called through dispatch_f_as masteru
static uint masteru_makes_subuhome(void *arg){
  char *subuhome = (char *) arg;
  if( mkdir( subuhome, 0700 ) == -1 ){
    perror("masteru_makes_subuhome");
    return ERR_SUBU_MK_0_MKDIR_SUBUHOME;
  }
  return 0;
}

subu_mk_0_ctx *subu_mk_0(sqlite3 *db, char *subuname){

  subu_mk_0_ctx *ctxp = subu_mk_0_ctx_mk();

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("Checking that subuname is well formed, counting its length.\n");
  #endif
  size_t subuname_len;
  {
    int ret = allowed_subuname(subuname, subuname_len);
    if( ret == -1 ){
      ctxp->err = ERR_SUBU_MK_0_SUBU_MALFORMED;
      ctxp->aux = subuname;
      return ctxp
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
      ctxp->err = ERR_SUBU_MK_0_SETUID_ROOT;
      return ctxp;
    }
  }

  //--------------------------------------------------------------------------------
  #ifdef DEBUG
  dbprintf("creation of required strings and recording their lengths\n");
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
    #ifdef DEBUG
    dbprintf("masteru_home \"%s\" %zu\n", masteru_home, masteru_home_len);
    #endif
    masteru_home_len = strlen(masteru_home);
    if( masteru_home_len == 0 || masteru_home[0] == '(' ){
      ctxp->err = ERR_SUBU_MK_0_MASTERU_HOMELESS;
      ctxp->aux = masteru_name;  // we can not free a passwd struct, or its fields.  I assume then it isn't re-entrant safe.
      return ctxp;
    }
    char *subuland_extension = "/subuland/";
    size_t subuland_extension_len = strlen(subuland_extension);
    ctxp->subuland = (char *)malloc( masteru_home_len + subuland_extension_len + 1 );
    if(!ctxp->subuland){
      ctxp->err = ERR_SUBU_MK_0_MALLOC;
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
  dbprintf("generate the subu_username, set the subuhome\n");
  #endif
  size_t subu_username_len;
  size_t subuhome_len;
  {
    char *ns=0;
    char *mess=0;
    if( subu_number_get( db, &ns, &mess ) != SQLITE_OK ){
      ctxp->err = ERR_SUBU_MK_0_CONFIG_FILE;
      ctxp->aux = mess;
      ctxp->free_aux = true;
      return ctxp;
    }
    ns_len = strlen(ns);
    ctxp->subu_username = malloc(1 + ns_len + 1);
    if( !ctxp->subu_username ){
      ctxp->err = ERR_SUBU_MK_0_MALLOC;
      return ctxp;
    }
    strcpy(ctxp->subu_username, "s");
    strcpy(ctxp->subu_username + 1, ns);
    subu_username_len = ns_len + 1;
    #ifdef DEBUG
    dbprintf("subu_username \"%s\" %zu\n", ctxp->subu_username, subu_username_len);
    #endif

    subuhome_len = subuland_len + subu_username_len; 
    ctxp->subuhome = (char *)malloc(subuhome_len + 1);
    if( !ctxp->subuhome ){
      ctxp->err = ERR_SUBU_MK_0_MALLOC;
      return ctxp;
    }
    strcpy (ctxp->subuhome, ctxp->subuland);
    strcpy (ctxp->subuhome + subuland_len, subu_username_len);
    subuhome_len = subuland_land_len + subu_username_len;
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
      ctxp->err = ERR_SUBU_MK_0_SUBUHOME_EXISTS;
      return ctxp;
    }
    int ret = dispatch_f_euid_egid
      (
       "masteru_makes_subuhome", 
       masteru_makes_subuhome, 
       (void *)ctxp->subuhome,
       masteru_uid, 
       masteru_gid
       );
    if( ret < 0 || ret == ERR_SUBU_MK_0_MKDIR_SUBUHOME ){
      ctxp->err = ERR_SUBU_MK_0_MKDIR_SUBUHOME;
      return ctxp;
    }
  }
  #ifdef DEBUG
  dbprintf("masteru made directory \"%s\"\n", ctxp->subuhome);
  #endif

  /*--------------------------------------------------------------------------------
    Make the subservient user account, i.e. the subu

  */
  {
    #ifdef DEBUG
      dbprintf("making subu \"%s\ as user \"%s\"\n", subuname, ctxp->subu_username);
    #endif
    #if BUG_SSS_CACHE_RUID
      #ifdef DEBUG
        dbprintf("setting inherited real uid to 0 to accomodate SSS_CACHE UID BUG\n");
      #endif
      if( setuid(0) == -1 ){
        ctxp->err = ERR_SUBU_MK_0_BUG_SSS;
        return ctxp;
      }
    #endif
    char *command = "/usr/sbin/useradd";
    char *argv[3];
    argv[0] = command;
    argv[1] = subu_username;
    argv[2] = (char *) NULL;
    char *envp[1];
    envp[0] = (char *) NULL;
    struct dispatch_useradd_ret_t ret;
    ret = dispatch_useradd(argv, envp);
    if(ret.error){
      ctxp->err = ERR_SUBU_MK_0_FAILED_USERADD;
      return ctxp;
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
    int ret = subu_put_masteru_subu(db, masteru_name, subuname, subu_username);
    if( rc != SQLITE_DONE ){
      ctxp->err = ERR_SUBU_MK_0_CONFIG_FILE;
      ctxp->aux = "insert of masteru subu relation failed";
      return ctxp;
    }
  }

  #ifdef DEBUG
  dbprintf("finished subu-mk-0(%s)\n", subuname);
  #endif
  RETURN_SUBU_MK_0(0);
}
