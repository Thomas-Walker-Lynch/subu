/*
There is no C library interface to useradd(8), but if there were, these functions
would be found there instead. I'm also wondering about the wisdom of using
useradd(8) from shadow.  Wondering if the locks are in place for multiple users hitting it
simultaneously via different methods, and if it handles signals cleanly.

We pass argv[1:..] and envp through to useradd.  We return the uid of the newly
created user, or the negative value of the return code from useradd(8)

*/

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

// better to change this to vargs for the option list
// this currently works because I know there is only one option
int useradd_0(char *user, char **argv, char **envp){
  char *subu_name;
  if( argv && argv[0] && argv[1] ){
    
  }
  pid_t pid = fork(void);
  if( pid == -1 ) return 1;
  if( pid == 0 ){ // we are the child
    char *command = "/usr/bin/useradd";
    argv[0] = command; // we can do this because we made the argv in the caller
    int err = execvpe( command, argv, envp);
    fprintf(stderr, "subu execvpe call to /usr/bin/useradd issued error %d, but it is too difficult to tell the parent process about this.", errno);
  } else {
    int wstatus;
    wait(1, &wstatus, 0);
    if(wstatus != 0) return -abs(wstatus);
    // useradd did not tell us the uid of the newly created user, so now we have go get it.
    struct passwd *getpwname(
  }

}
