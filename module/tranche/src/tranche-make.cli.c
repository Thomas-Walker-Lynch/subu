/*

usage: 
   argv[0] [<source_file_path>] [-sname <sname>] [-sdir <dir>] [-tdir <dir>] [-mfile <mfile_path>]

gets the names of all the targets from the source file, then appends
to the mfile a couple of lines of the form:

<target>...  : <src>
      tranche $@

Our makefile is typically run in the directory above where the sources are
located.

options
<source_file_path>  the trc file to be read
-sdir <sdir>   prepend <sdir> to <src> in the makefile deps line that is printed
-tdir <tdir>   prepend <tdir> to each <target> "
-mfile <mfile_path> where to send the output, defaults to stdout
-sname <sname> replaces sourcename as the name to write as the source - useful for pipes

If <source_file_path> is not provided stdin is used.
If -mfile is not provided, stdout is used. 
If -sdir is not provided, the directory part of the <source_file_path> is used. If the
user does not want this behavior, give a value of "." for -sdir.

*/
  
#include <stdio.h>
#include <unistd.h>
#include <da.h>
#include "tranche.lib.h"

int main(int argc, char **argv, char **envp){

  char *source_file_path = 0;
  char *sname = 0;
  char *sdir = 0;  
  char *tdir = 0;  
  char *mfile_path = 0; 

  { // argument parse
    Da args; // we will queue the non option args here
    Da *argsp = &args;
    da_alloc(argsp, sizeof(char *));
    int args_cnt = 0;

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
          continue;
        }
        pt++; if(!*pt || **pt == '-'){
          fprintf(stderr, "Missing value for option %s.\n", option);
          err_cnt++;
          if(!*pt) break;
          continue;
        }
        value = *pt;
        if( !strcmp(option, "mfile") ){
          mfile_path = value;
          continue;
        }
        if( !strcmp(option, "sdir") ){
          sdir = value;
          continue;
        }
        if( !strcmp(option, "tdir") ){
          tdir = value;
          continue;
        }
        if( !strcmp(option, "sname") ){
          sname = value;
          continue;
        }
        fprintf(stderr, "Unrecognized option %s.", option);
        err_cnt++;
        continue;
      }
      da_push(argsp, pt);
      arg_cnt++;
      pt++;
    }
    if(!da_emptyq(argsp)) src_file_path = da_index(args, 0);

    // arg contracts
    if(da_length(argsp) > 1){
      fprintf(stderr, "too many args/n");
      err_cnt++;
    }
    if(!src_file_name && !sname){
      fprintf(stderr, "must specify at least one eof a source_file_path or an sname/n");
      err_cnt++;
    }
    if(err_cnt > 0){
      fprintf(stderr, "usage: %s [<source_file_path>] [-sname <sname>] [-sdir <dir>] [-tdir <dir>] [-mfile <mfile_path>]\n", argv[0]);
      return TRANCHE_ERR_ARG_PARSE;
    }

  }// end of argument parse
 
  FILE *src_file;
  int mfile_fd;
  { //source and mfile open
    if(!src_file_path)
      src_file = stdin;
    else
      src_file = fopen(src_file_path, "r");

    if(mfile_path == "")
      mfile_fd = STDOUT_FILENO;
    else
      mfile_fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0666);

    int err = 0;
    if(!src_file){
      fprintf(stderr,"could not open tranche source file %s\n", src_file_path);
      err+= ERR_SRC_OPEN;
    }
    if(mfile_fd == -1){
      fprintf(stderr, "Could not open the dep file %s\n", mfile_path);
      err+= ERR_DST_OPEN;
    }
    if(err) return err;
  }

  char *file_name_part;
  if(src_file_path){
    // we are guaranteed a filename part, otherwise the fopen above would have failed
    file_name_part = path_chop(src_file_path);
    if(!sname) sname = file_name_part;
    if(!sdir && file_name_part != src_file_path) sdir = src_file_path; // note the file name has been chopped from src_file_path
  }
  tranche_make(src_file, sname,  mfile_fd, sdir, tdir);

  { // deallocate resources instead of just existing, so as to catch any errors
    da_free(argsp);
    int err_cnt = 0;
    if(src_file != stdin)
      if( !fclose(src_file) ){perror(); err_cnt++;}
    if(mfile_fd != STDOUT_FILENO)
      if( close(mfile_fd) == -1 ){perror(); err_cnt++;}
    if( err_cnt )
      return TRANCHE_ERR_CLOSE;
    else
      return 0;
  }
}
