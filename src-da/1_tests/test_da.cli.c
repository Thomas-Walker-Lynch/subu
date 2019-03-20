
#include <stdio.h>
#include <stdbool.h>
#include "test_da.lib.h"

int main(){
  bool da_0_passed = test_da_0();

  unsigned int passed = 0;
  unsigned int failed = 0;

  if( !test_da_0() ){
    failed++;
    printf("test_da_0 failed\n");
  }else
    passed++;

  if( !test_da_1() ){
    failed++;
    printf("test_da_1 failed\n");
  }else
    passed++;

  if( passed == 0 && failed == 0)
    printf("no tests ran\n");
  else if( passed == 0 )
    printf("failed all %u tests\n", failed);
  else if( failed == 0 )
    printf("passed all %u tests\n", passed);
  else
    printf("failed %u of %u tests\n", failed, passed + failed);

  if( passed == 0 || failed != 0 ) return 1;
  return 0;
}
