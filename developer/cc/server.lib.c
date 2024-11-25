#ifndef IFACE
#define Server·IMPLEMENTATION
#define IFACE
#endif

#ifndef Server·IFACE
#define Server·IFACE

  // Interface prototype
  int Server·run();

#endif // Server·IFACE

#ifdef Server·IMPLEMENTATION

  // Implementation-specific includes
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <stddef.h>
  #include <sys/types.h>
  #include <bits/socket.h>  // Ensure full definition of struct ucred
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>

  // Type alias for ucred
  typedef struct ucred ucred_t;

  // Constants
  #define Server·SOCKET_PATH "/var/user_data/Thomas-developer/subu/developer/mockup/subu_server_home/subu_server.sock"
  #define Server·LOG_PATH "server.log"
  #define Server·BUFFER_SIZE 256
  #define MAX_ARGC 16

  // Internal function prototypes
  static void parse(const ucred_t *client_cred, const char *input_line);
  static void hello(const ucred_t *client_cred, int argc, char *argv[]);

  int Server·run(){
    int server_fd, client_fd;
    struct sockaddr_un address;
    char buffer[Server·BUFFER_SIZE];
    FILE *log_file;

    log_file = fopen(Server·LOG_PATH, "a+");
    if(log_file == NULL) {
      perror("Server·run:: error opening log file");
      return EXIT_FAILURE;
    }

    if((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      perror("Server·run:: error creating socket");
      fclose(log_file);
      return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, Server·SOCKET_PATH, sizeof(address.sun_path) - 1);

    unlink(Server·SOCKET_PATH);
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
      perror("Server·run:: error binding socket");
      fclose(log_file);
      close(server_fd);
      return EXIT_FAILURE;
    }

    if(listen(server_fd, 5) == -1) {
      perror("Server·run:: error listening on socket");
      fclose(log_file);
      close(server_fd);
      return EXIT_FAILURE;
    }

    printf("Server·run:: server running, waiting for connections...\n");

    while((client_fd = accept(server_fd, NULL, NULL)) != -1) {
      ucred_t client_cred;
      socklen_t len = sizeof(client_cred);

      if(getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &client_cred, &len) == -1) {
        perror("Server·run:: error getting client credentials");
        close(client_fd);
        continue;
      }

      // Log client credentials
      fprintf(log_file, "Connection from PID=%d, UID=%d, GID=%d\n",
              client_cred.pid, client_cred.uid, client_cred.gid);
      fflush(log_file);

      ssize_t bytes_read;
      memset(buffer, 0, Server·BUFFER_SIZE);
      bytes_read = read(client_fd, buffer, Server·BUFFER_SIZE - 1);
      if(bytes_read > 0) {
        printf("Server·run:: received: %s\n", buffer);
        fprintf(log_file, "Received: %s\n", buffer);
        fflush(log_file);

        char *line = strtok(buffer, "\n");
        while(line != NULL) {
          parse(&client_cred, line);
          line = strtok(NULL, "\n");
        }
      } else if(bytes_read == -1) {
        perror("Server·run:: error reading from client");
      }

      close(client_fd);
    }

    perror("Server·run:: error accepting connection");
    fclose(log_file);
    close(server_fd);
    unlink(Server·SOCKET_PATH);

    return EXIT_FAILURE;
  }

  // Parse a single input line and dispatch to the appropriate command
  static void parse(const ucred_t *client_cred, const char *input_line) {
    char *argv[MAX_ARGC + 1] = {0};
    int argc = 0;

    char *line_copy = strdup(input_line);
    if(!line_copy) {
      perror("parse:: memory allocation failed");
      return;
    }

    char *token = strtok(line_copy, " ");
    while(token != NULL && argc < MAX_ARGC) {
      argv[argc++] = token;
      token = strtok(NULL, " ");
    }

    if(argc > 0) {
      if(strcmp(argv[0], "hello") == 0) {
        hello(client_cred, argc, argv);
      }else{
        fprintf(stderr, "Unknown command: %s\n", argv[0]);
      }
    }

    free(line_copy);
  }

  // Example command: hello
  static void hello(const ucred_t *client_cred, int argc, char *argv[]) {
    printf("hello:: invoked by PID=%d, UID=%d, GID=%d\n",
           client_cred->pid, client_cred->uid, client_cred->gid);
    printf("hello:: arguments:\n");
    for (int i = 1; i < argc; ++i) {
      printf("  Arg %d: %s\n", i, argv[i]);
    }
  }

#endif // Server·IMPLEMENTATION
