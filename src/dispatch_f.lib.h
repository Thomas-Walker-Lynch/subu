#ifndef DISPATCH_LIB_H
#define DISPATCH_LIB_H
#include "local_common.h"

#define ERR_FORK -1;
#define ERR_SETEUID -2;
#define ERR_SETEGID -3;

int dispatch_f(char *fname, int (*f)());
int dispatch_f_euid_egid(char *fname, int (*f)(), uid_t euid, gid_t egid);

#endif


