#include "x86_socket.h"
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include "misc.h"
#include <stdlib.h>
#include "log.h"

#define X86_SOCKET_INVALID_FD -1
#define X86_SOCKET_BUFFER_LEN 1024

// glibc - http://man7.org/linux/man-pages/man3/program_invocation_name.3.html
extern char *program_invocation_short_name;

static StatusCode prv_setup_socket(X86SocketThread *thread, int *server_fd) {
  // Create local socket
  *server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (*server_fd < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Failed to create socket");
  }

  struct sockaddr_un addr = {
    .sun_family = AF_UNIX
  };
  // First character is \0 to signal abstract domain socket
  snprintf(addr.sun_path + 1, sizeof(addr.sun_path) - 1, "%d/%s/%s", 0 /* getpid() */, program_invocation_short_name, thread->module_name);

  if (bind(*server_fd, (struct sockaddr *)&addr, offsetof(struct sockaddr_un, sun_path) + 1 + strlen(addr.sun_path + 1)) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Failed to bind socket");
  }

  if (listen(*server_fd, X86_SOCKET_MAX_PENDING_CONNECTIONS) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Failed to specify maximum pending connections");
  }

  return STATUS_CODE_OK;
}

static void *prv_server_thread(void *context) {
  X86SocketThread *thread = context;
  int server_fd = -1;
  if (!status_ok(prv_setup_socket(thread, &server_fd))) {
    LOG_DEBUG("Socket setup failed\n");
    return NULL;
  }

  LOG_DEBUG("Started RX server for %s\n", thread->module_name);

  // Mutex unlocked when thread should exit
  while (pthread_mutex_trylock(&thread->keep_alive) != 0) {
    // Build new fd set - select will mangle it
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    int max_fd = server_fd;

    for (int i = 0; i < X86_SOCKET_MAX_CLIENTS; i++) {
      if (thread->client_fds[i] != X86_SOCKET_INVALID_FD) {
        FD_SET(thread->client_fds[i], &read_fds);
        max_fd = MAX(max_fd, thread->client_fds[i]);
      }
    }

    select(max_fd + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(server_fd, &read_fds)) {
      // Server read - new client
      int new_socket = accept(server_fd, NULL, NULL);
      if (new_socket < 0) {
        LOG_DEBUG("Failed to accept new client!\n");
      }
      LOG_DEBUG("New client %d connected to %s!\n", new_socket, thread->module_name);

      for (size_t i = 0; i < X86_SOCKET_MAX_CLIENTS; i++) {
        if (thread->client_fds[i] == X86_SOCKET_INVALID_FD) {
          thread->client_fds[i] = new_socket;
          break;
        }
      }
    }

    // Handle client reads - expect packets prefixed with a 32-bit unsigned integer representing
    // packet size
    for (size_t i = 0; i < X86_SOCKET_MAX_CLIENTS; i++) {
      int client_fd = thread->client_fds[i];
      if (client_fd != X86_SOCKET_INVALID_FD && FD_ISSET(client_fd, &read_fds)) {
        char buffer[X86_SOCKET_BUFFER_LEN] = { 0 };
        ssize_t read_len = read(client_fd, buffer, X86_SOCKET_BUFFER_LEN);
        if (read_len <= 0) {
          // Disconnected
          LOG_DEBUG("Client %d disconnected from %s\n", client_fd, thread->module_name);
          close(client_fd);
          thread->client_fds[i] = X86_SOCKET_INVALID_FD;
        } else {
          thread->handler(thread, client_fd, buffer, (size_t)read_len, thread->context);
        }
      }
    }
  }

  return NULL;
}

StatusCode x86_socket_init(X86SocketThread *thread, char *module_name, X86SocketHandler handler, void *context) {
  memset(thread, 0, sizeof(*thread));
  thread->module_name = module_name;
  thread->handler = handler;
  thread->context = context;

  for (size_t i = 0; i < X86_SOCKET_MAX_CLIENTS; i++) {
    thread->client_fds[i] = X86_SOCKET_INVALID_FD;
  }

  pthread_mutex_init(&thread->client_lock, NULL);
  // Mutex is used as cancellation point for thread - locked = keep alive
  pthread_mutex_init(&thread->keep_alive, NULL);
  pthread_mutex_lock(&thread->keep_alive);

  pthread_create(&thread->thread, NULL, prv_server_thread, thread);

  return STATUS_CODE_OK;
}

StatusCode x86_socket_broadcast(X86SocketThread *thread, const char *tx_data, size_t tx_len) {
  // TODO: does this need to be protected?
  pthread_mutex_lock(&thread->client_lock);
  for (size_t i = 0; i < X86_SOCKET_MAX_CLIENTS; i++) {
    if (thread->client_fds[i] != X86_SOCKET_INVALID_FD) {
      ssize_t write_len = write(thread->client_fds[i], tx_data, tx_len);
    }
  }
  pthread_mutex_unlock(&thread->client_lock);

  return STATUS_CODE_OK;
}
