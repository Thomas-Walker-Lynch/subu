
#include <stdio.h>
#include <stdbool.h>
#include "test_da.lib.h"
#include <da.h>
#include <acc.h>

//AccChannel acc_live_channels;

int main(){
  //acc_open(&acc_live_channels, acc_SELF);
    // enumeration of tests
  typedef bool (*test_fun)();
  test_fun tests[] =
    {
      test_da_push_0,
      test_da_expand_0,
      test_da_string_input_0,
      test_da_pop_0,
      test_da_cat_0,
      test_da_cat_1,
      test_da_rewind_0,
      test_da_index_0,
      test_da_free_elements_0,
      test_da_strings_exists_0,
      test_da_rebase_0,
      test_da_boundq_0,
      test_da_foreach_0,
      //test_da_present_0,
      test_da_exists_0,
      test_da_exists_1,
      //test_da_all_0,
      test_da_init_0,
      test_da_free_0,
      test_da_is_empty_0,
      test_da_length_0,
      // test_da_push_row_0,
      // test_da_erase_0,
      // test_da_longer_0,
      // test_da_longest_0,
      test_da_accounting_0,
      NULL};
  char *test_names[] =
    {
      "test_da_push_0",
      "test_da_expand_0",
      "test_da_string_input_0",
      "test_da_pop_0",
      "test_da_cat_0",
      "test_da_cat_1",
      "test_da_rewind_0",
      "test_da_index_0",
      "test_da_free_elements_0",
      "test_da_strings_exists_0",
      "test_da_rebase_0",
      "test_da_boundq_0",
      "test_da_foreach_0",
      //"test_da_present_0",
      "test_da_exists_0",
      "test_da_exists_1",
      "test_da_all_0",
      "test_da_init_0",
      "test_da_free_0",
      "test_da_is_empty_0",
      "test_da_length_0",
      //"test_da_push_row_0",
      //"test_da_erase_0",
      //"test_da_longer_0",
      //"test_da_longest_0",
      "test_da_accounting_0",
      NULL};
  // call tests
  bool da_0_passed = true;
  unsigned int passed = 0;
  unsigned int failed = 0;
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
  //acc_report(&acc_live_channels);
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
