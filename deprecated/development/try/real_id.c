
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main(){
  int uid = getuid();
  int euid = geteuid();
  printf("real_id: %u effective_id: %u\n",uid,euid);
  return 0;
}
