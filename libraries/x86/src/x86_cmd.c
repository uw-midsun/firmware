#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define X86_CMD_MAX_CLIENTS 10
#define X86_CMD_MAX_PENDING_CONNS 3
#define X86_CMD_PORT 4000
#define X86_CMD_BUF_LEN 1024

void *prv_serve_thread(void *unused) {
  bool opt = true;
  int server_fd = 0;
  int addrlen = 0;
  int max_fd = 0;
  int client_fds[X86_CMD_MAX_CLIENTS] = { 0 };
  struct sockaddr_in addr = { 0 };
  char buf[X86_CMD_BUF_LEN + 1] = { 0 };  // Extra for null terminator

  fd_set readfds = { 0 };

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    printf("Failed to create socket!\n");
    return NULL;
  }

  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(X86_CMD_PORT);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Failed to bind socket!\n");
    return NULL;
  }

  if (listen(server_fd, X86_CMD_MAX_PENDING_CONNS) < 0) {
    printf("Failed to specify maximum pending connections\n");
    return NULL;
  }

  addrlen = sizeof(addr);

  while (true) {
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    max_fd = server_fd;

    for (int i = 0; i < X86_CMD_MAX_CLIENTS; i++) {
      if (client_fds[i] > 0) {
        FD_SET(client_fds[i], &readfds);
      }

      if (client_fds[i] > max_fd) {
        max_fd = client_fds[i];
      }
    }

    // Infinite timeout
    select(max_fd + 1, &readfds, NULL, NULL, NULL);

    if (FD_ISSET(server_fd, &readfds)) {
      // Server read - new client
      int new_socket = accept(server_fd, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
      if (new_socket < 0) {
        printf("Failed to accept new client!\n");
      }
      printf("New client %d from %s:%d connected!\n", new_socket, inet_ntoa(addr.sin_addr),
             ntohs(addr.sin_port));

      for (int i = 0; i < X86_CMD_MAX_CLIENTS; i++) {
        if (client_fds[i] == 0) {
          client_fds[i] = new_socket;
        }
      }
    }

    for (int i = 0; i < X86_CMD_MAX_CLIENTS; i++) {
      int client_fd = client_fds[i];
      if (FD_ISSET(client_fd, &readfds)) {
        ssize_t read_len = read(client_fd, buf, X86_CMD_BUF_LEN);
        if (read_len == 0) {
          // Disconnected
          printf("Client disconnected\n");
          close(client_fd);
          client_fds[i] = 0;
        } else if (read_len > 0) {
          // Replace newline with null terminator
          buf[read_len - 1] = '\0';
          if (buf[read_len - 2] == 13) {
            buf[read_len - 2] = '\0';
          }
          printf("RX %s\n", buf);
        }
      }
    }
  }

  return NULL;
}

void __attribute__((constructor)) x86_cmd_init(void) {
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, prv_serve_thread, NULL);
}
