/*
 * This command implements a simple Unix domain socket server.
 * 
 * Usage:
 *   server
 *
 * Description:
 *   This program creates a Unix domain socket at the specified path
 *   (defined in `server.lib.c`) and listens for incoming connections. 
 *   When a client sends data, the server logs it to a file and echoes 
 *   a receipt. The server runs until interrupted or an error occurs.
 * 
 * Command-line Options and Arguments:
 *   None: The program is self-contained and does not accept any
 *   command-line arguments. All configuration is handled within
 *   the implementation.
 */

#define IFACE
#include <stdio.h>
#include <stdlib.h>
#include "server.lib.c"

int main(int argc, char *argv[]) {
  (void)argc; // Suppress unused variable warnings
  (void)argv;

  // Call the core server function
  return Server·run();
}
