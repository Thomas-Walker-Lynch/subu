
#ifndef USERADD_H
#define USERADD_H
#include local_common.h

struct uid_gid{
  uint error;
  pid_t uid;
  pid_t gid;
};

uit_gid useradd_0(char *user, char **argv, char **envp);

#endif
