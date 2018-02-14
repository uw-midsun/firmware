#include "x86_tcp.h"
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

#define X86_TCP_PORT 4000
#define X86_TCP_BUF_LEN 1024
#define X86_TCP_MAX_CLIENTS 10
#define X86_TCP_MAX_PENDING_CONNECTIONS 3

static int prv_setup_socket(void) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    printf("Failed to create socket\n");
    return -1;
  }

  bool opt = true;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_addr.s_addr = INADDR_ANY,
                             .sin_port = htons(X86_TCP_PORT),
                             .sin_zero = { 0 } };
  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Failed to bind socket\n");
    return -1;
  }

  if (listen(server_fd, X86_TCP_MAX_PENDING_CONNECTIONS) < 0) {
    printf("Failed to specify maximum pending connections\n");
    return -1;
  }

  return server_fd;
}

static void *prv_serve_thread(void *arg) {
  X86TcpParser parser = arg;
  int server_fd = prv_setup_socket();
  int client_fds[X86_TCP_MAX_CLIENTS] = { 0 };
  char buf[X86_TCP_BUF_LEN + 1] = { 0 };  // Extra for null terminator

  while (true) {
    fd_set readfds = { 0 };
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    int max_fd = server_fd;

    for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
      if (client_fds[i] > 0) {
        FD_SET(client_fds[i], &readfds);
      }

      if (client_fds[i] > max_fd) {
        max_fd = client_fds[i];
      }
    }

    // Infinite timeout
    select(max_fd + 1, &readfds, NULL, NULL, NULL);

    // Server read - new client
    if (FD_ISSET(server_fd, &readfds)) {
      struct sockaddr_in addr = { 0 };
      socklen_t addrlen = sizeof(addr);
      int new_socket = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
      if (new_socket < 0) {
        printf("Failed to accept new client!\n");
      }
      printf("New client %d from %s:%d connected!\n", new_socket, inet_ntoa(addr.sin_addr),
             ntohs(addr.sin_port));

      for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
        if (client_fds[i] == 0) {
          client_fds[i] = new_socket;
          break;
        }
      }
    }

    // Handle client reads - incoming data or disconnect
    for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
      int client_fd = client_fds[i];
      if (FD_ISSET(client_fd, &readfds)) {
        ssize_t read_len = read(client_fd, buf, X86_TCP_BUF_LEN);

        if (read_len == 0) {
          // Disconnected
          printf("Client disconnected\n");

          ssize_t write_len = write(client_fd, "Disconnecting\n", 13);
          close(client_fd);
          client_fds[i] = 0;
        } else if (read_len > 0) {
          // Replace newline with null terminator
          buf[read_len - 1] = '\0';
          if (buf[read_len - 2] == 13) {
            buf[read_len - 2] = '\0';
          }

          if (parser != NULL) {
            parser(buf);
          }
          ssize_t write_len = write(client_fd, "Success\n", 7);
        }
      }
    }
  }

  return NULL;
}

void x86_tcp_init(X86TcpParser parser) {
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, prv_serve_thread, parser);
}
