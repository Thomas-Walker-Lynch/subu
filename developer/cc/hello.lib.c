#ifndef IFACE
#define Hello·IMPLEMENTATION
#define IFACE
#endif

#ifndef Hello·IFACE
#define Hello·IFACE

  // Necessary interface includes
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <stddef.h>

  // Interface prototypes
  int Hello·run();

#endif // Hello·IFACE

#ifdef Hello·IMPLEMENTATION

  // Implementation-specific includes
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>

  // Constants
  #define Hello·SOCKET_PATH "/var/user_data/Thomas-developer/subu/developer/mockup/subu_server_home/subu_server.sock"
  #define Hello·LOG_PATH "server_test.log"
  #define Hello·BUFFER_SIZE 256

  int Hello·run(){
    int client_fd;
    struct sockaddr_un address;
    char buffer[Hello·BUFFER_SIZE];
    FILE *log_file;

    // Open the log file
    log_file = fopen(Hello·LOG_PATH ,"a+");
    if( log_file == NULL ){
      perror("Hello·run:: error opening log file");
      return EXIT_FAILURE;
    }

    client_fd = socket(AF_UNIX ,SOCK_STREAM ,0);
    if( client_fd == -1 ){
      perror("Hello·run:: error opening socket");
      fclose(log_file);
      return EXIT_FAILURE;
    }

    // Configure server socket address
    memset(&address ,0 ,sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path ,Hello·SOCKET_PATH ,sizeof(address.sun_path) - 1);

    // Connect to the server
    if( connect(client_fd ,(struct sockaddr *)&address ,sizeof(address)) == -1 ){
      perror("Hello·run:: error connecting to server");
      fclose(log_file);
      close(client_fd);
      return EXIT_FAILURE;
    }

    // Send message to the server
    char *out_buf = "hello\n";
    if( write(client_fd ,out_buf ,strlen(out_buf)) == -1 ){
      perror("Hello·run:: error writing to server");
      fclose(log_file);
      close(client_fd);
      return EXIT_FAILURE;
    }

    printf("Hello·run:: sent \"%s\"\n" ,out_buf);

    // Clean up
    fclose(log_file);
    close(client_fd);

    return EXIT_SUCCESS;
  }

#endif // Hello·IMPLEMENTATION
