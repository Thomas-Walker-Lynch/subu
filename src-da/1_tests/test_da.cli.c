
#include <stdio.h>
#include <stdbool.h>
#include "test_da.lib.h"

int main(){
  bool da_0_passed = test_da_0();

  unsigned int passed = 0;
  unsigned int failed = 0;

  // enumeration of tests
  typedef bool (*test_fun)();
  test_fun tests[] = {test_da_0, test_da_1, test_da_2, test_da_3, NULL};
  char *test_names[] = {"test_da_0", "test_da_1", "test_da_2", "test_da_3", NULL};

  // call tests
  test_fun *tfp = tests;
  char **tnp = test_names;
  while(*tfp){
    if( !(*tfp)() ){
      failed++;
      if(*tnp)
        printf("%s failed\n", *tnp);
      else
        fprintf(stderr, "internal error, no test_names[] entry for test\n");
    }else
      passed++;
  tfp++;
  tnp++;
  }

  // summarize results

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
