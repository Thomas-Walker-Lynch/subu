#ifndef IFACE
#define Server·IMPLEMENTATION
#define IFACE
#endif

#ifndef Server·IFACE
#define Server·IFACE

  // Necessary interface includes
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <stddef.h>

  // Interface prototypes
  int Server·run();

#endif // Server·IFACE

#ifdef Server·IMPLEMENTATION

  // Implementation-specific includes
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>

  // Constants
  #define Server·SOCKET_PATH "mockup/subu_server_home/subu_server.sock"
  #define Server·LOG_PATH "mockup/subu_server_home/server.log"
  #define Server·BUFFER_SIZE 256

  int Server·run() {
    int server_fd, client_fd;
    struct sockaddr_un address;
    char buffer[Server·BUFFER_SIZE];
    FILE *log_file;

    // Open the log file
    log_file = fopen(Server·LOG_PATH, "a");
    if (log_file == NULL) {
      perror("Error opening log file");
      return EXIT_FAILURE;
    }

    // Create the socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      perror("Error creating socket");
      fclose(log_file);
      return EXIT_FAILURE;
    }

    // Configure socket address
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, Server·SOCKET_PATH, sizeof(address.sun_path) - 1);

    // Bind the socket
    unlink(Server·SOCKET_PATH); // Remove existing file if present
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
      perror("Error binding socket");
      fclose(log_file);
      close(server_fd);
      return EXIT_FAILURE;
    }

    // Listen for connections
    if (listen(server_fd, 5) == -1) {
      perror("Error listening on socket");
      fclose(log_file);
      close(server_fd);
      return EXIT_FAILURE;
    }

    printf("Server running, waiting for connections...\n");

    // Accept and handle client connections
    while ((client_fd = accept(server_fd, NULL, NULL)) != -1) {
      ssize_t bytes_read;

      memset(buffer, 0, Server·BUFFER_SIZE);
      bytes_read = read(client_fd, buffer, Server·BUFFER_SIZE - 1);
      if (bytes_read > 0) {
        fprintf(log_file, "Received: %s\n", buffer);
        fflush(log_file);
      } else if (bytes_read == -1) {
        perror("Error reading from client");
      }

      close(client_fd);
    }

    // Clean up
    perror("Error accepting connection");
    fclose(log_file);
    close(server_fd);
    unlink(Server·SOCKET_PATH);

    return EXIT_FAILURE;
  }

#endif // Server·IMPLEMENTATION
