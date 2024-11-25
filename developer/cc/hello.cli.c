/*
  Sends "hello" to `subu_server.sock`
*/

#define IFACE
#include <stdio.h>
#include <stdlib.h>
#include "hello.lib.c"

int main(int argc, char *argv[]) {
  (void)argc; // Suppress unused variable warnings
  (void)argv;

  // Call the core server function
  return Hello·run();
}
