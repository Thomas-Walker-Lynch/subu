/*

usage: 
   argv[0] [<src_file_path>] [-sname <sname>] [-tdir <tdir>] [-mfile <mfile_path>]

gets the names of all the targets from the source file, then appends
to the mfile a couple of lines of the form:

<target>...  : <src>
      tranche $@

Our makefile is typically run in the directory above where the sources are
located.

options
<src_file_path>  the trc file to be read
-tdir <tdir>   prepend <tdir>/ to each <target> "
-mfile <mfile_path> where to send the output, defaults to stdout
-sname <sname> replaces sourcename as the name to write as the source - useful for pipes

If <src_file_path> is not provided stdin is used.
If -mfile is not provided, stdout is used. 
If -sdir is not provided, the directory part of the <src_file_path> is used. If the
user does not want this behavior, give a value of "." for -sdir.

.. should modify this to allow multiple source files on the command line ..
.. should check that tdir is an existing directory
*/
  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <da.h>
#include "tranche.lib.h"

int main(int argc, char **argv, char **envp){

  char *src_file_path = 0;
  char *sname = 0;
  char *tdir = 0;  
  char *mfile_path = 0; 

  { // argument parse
    Da args; // we will queue the non option args here
    Da *argsp = &args;
    da_alloc(argsp, sizeof(char *));

    int err_cnt = 0;
    char **pt = argv;
    char *option;
    char *value; // currently all our tranche options have exactly one value
    while(*pt){
      if( **pt == '-' ){
        option = *pt + 1;
        if(!*option){
          fprintf(stderr, "Currently there is no lone '-' option.\n");
          err_cnt++;
          goto endif;
        }
        pt++; if(!*pt || **pt == '-'){
          fprintf(stderr, "Missing value for option %s.\n", option);
          err_cnt++;
          if(!*pt) break; // then nothing more to parse
          goto endif;
        }
        value = *pt;
        if( !strcmp(option, "mfile") ){
          mfile_path = value;
          goto endif;
        }
        if( !strcmp(option, "tdir") ){
          tdir = value;
          goto endif;
        }
        if( !strcmp(option, "sname") ){
          sname = value;
          goto endif;
        }
        fprintf(stderr, "Unrecognized option %s.", option);
        err_cnt++;
        goto endif;
      }
      // else clause
      da_push(argsp, pt);
    endif:
    pt++;
    }
    int args_cnt = da_length(argsp);
    if(args_cnt > 1) src_file_path = *(char **)da_index(argsp, 1);

    // arg contracts
    if(args_cnt > 2){
      fprintf(stderr, "too many args\n");
      err_cnt++;
    }
    if(!src_file_path && !sname){
      fprintf(stderr, "must specify at least one of a src_file_path or an sname\n");
      err_cnt++;
    }
    if(err_cnt > 0){
      fprintf(stderr, "usage: %s [<src_file_path>] [-sname <sname>] [-tdir <dir>] [-mfile <mfile_path>]\n", argv[0]);
      return TRANCHE_ERR_ARG_PARSE;
    }
    da_free(argsp); // this only frees the array itself, not the things it points to

  }// end of argument parse
 
  FILE *src_file;
  int mfile_fd;
  { //source and mfile open
    if(!src_file_path)
      src_file = stdin;
    else
      src_file = fopen(src_file_path, "r");

    if(mfile_path)
      mfile_fd = open(mfile_path, O_WRONLY | O_APPEND | O_CREAT, 0666);
    else
      mfile_fd = STDOUT_FILENO;

    int err = 0;
    if(!src_file){
      fprintf(stderr,"could not open tranche source file %s\n", src_file_path);
      err+= TRANCHE_ERR_SRC_OPEN;
    }
    if(mfile_fd == -1){
      fprintf(stderr, "Could not open the dep file %s\n", mfile_path);
      err+= TRANCHE_ERR_DST_OPEN;
    }
    if(err) return err;
  }

  if(sname)src_file_path = sname;
  path_trim_slashes(tdir);
  tranche_make(src_file, src_file_path,  mfile_fd, tdir);

  { // deallocate resources instead of just existing, so as to catch any errors
    int err_cnt = 0;
    if(src_file != stdin)
      if( fclose(src_file) ){perror(NULL); err_cnt++;}
    if(mfile_fd != STDOUT_FILENO)
      if( close(mfile_fd) == -1 ){perror(NULL); err_cnt++;}
    if( err_cnt )
      return TRANCHE_ERR_FCLOSE;
    else
      return 0;
  }
}
