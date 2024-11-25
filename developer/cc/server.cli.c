/*
  The subu server command line interface.

  Usage:
    server [-s <socket_path>] [-l <log_path>] [arguments...]

  Options:
    -s <socket_path>  Specify the Unix socket file path. Default: ./socket
    -l <log_path>     Specify the log file path. Default: ./log.txt
*/

#define IFACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "server.lib.c"

// Define defaults
#define DEFAULT_SOCKET_PATH "socket"
#define DEFAULT_LOG_PATH "log.txt"

int main( int argc ,char *argv[] ){
  char *socket_path = DEFAULT_SOCKET_PATH;
  char *log_path = DEFAULT_LOG_PATH;
  int error_flag = 0; // Flag to track errors

  // Parse options
  int opt;
  while( (opt = getopt(argc ,argv ,"s:l:")) != -1 ){
    switch( opt ){
      case 's':
        socket_path = optarg;
        break;
      case 'l':
        log_path = optarg;
        break;
      case '?': // Unknown option
        fprintf( stderr ,"Error: Unknown option '-%c'\n" ,optopt );
        error_flag = 1;
        break;
      case ':': // Missing argument
        fprintf( stderr ,"Error: Missing argument for '-%c'\n" ,optopt );
        error_flag = 1;
        break;
    }
  }

  // Check for too many arguments
  if( optind > argc - 1 ){
    fprintf( stderr ,"Error: Too many arguments provided.\n" );
    error_flag = 1;
  }

  // Exit on error after processing all options
  if( error_flag ){
    fprintf( stderr ,"Usage: %s [-s <socket_path>] [-l <log_path>] [arguments...]\n" ,argv[0] );
    return EXIT_FAILURE;
  }

  // Rebase argv to prepare for run
  argv[optind - 1] = argv[0]; // Program name at the new base
  argc -= (optind - 1);
  argv += (optind - 1);

  // Log parsed options
  printf( "Socket Path: %s\n" ,socket_path );
  printf( "Log Path: %s\n" ,log_path );

  // Call the core server function with the rebased arguments
  return Server·run(argv ,argc ,socket_path ,log_path);
}
