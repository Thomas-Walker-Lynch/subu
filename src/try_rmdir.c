
#include <stdio.h>
#include <unistd.h>
int main(){
  int retval = rmdir("/home/morpheus/subuland/ttemp0");
  printf("retval %d\n",retval);
  return 0;
}
