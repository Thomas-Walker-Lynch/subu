#ifndef RUN_FI_H
#define RUN_FI_H

#include "local_common.h"

#define ADMIN_ERR_NO_COMMAND 1
#define ADMIN_ERR_FORK 2
#define ADMIN_ERR_ARGC 3

struct run_err_struct{
  int admin; // admin error
  int exec;  // errno from failed exec call 
  int process; // err code returned from process as reported by wait()
};
inline run_err_init(struct run_err_struct re){
  re.admin = 0;
  re.exec = 0;
  re.process = 0;
}
inline run_err_has(struct run_err_struct re){
  return re.admin || re.exec || re.process 
}

extern struct run_err_struct run_err;
uint run(char **argv, char **envp);

#endif


