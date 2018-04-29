#include "x86_tcp.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include "misc.h"
#include <stdlib.h>
#include "log.h"

#define X86_TCP_INVALID_FD -1

static StatusCode prv_setup_socket(int *server_fd) {
  *server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (*server_fd == 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Failed to create socket");
  }

  bool opt = true;
  setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

  struct sockaddr_in addr = {
    .sin_family = AF_INET, //
    .sin_addr.s_addr = INADDR_ANY, //
    .sin_port = 0, // Random port
    .sin_zero = { 0 } //
  };

  if (bind(*server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Failed to bind socket");
  }

  if (listen(*server_fd, X86_TCP_MAX_PENDING_CONNECTIONS) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Failed to specify maximum pending connections");
  }

  return STATUS_CODE_OK;
}

static int prv_get_port(int server_fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  getsockname(server_fd, (struct sockaddr *)&addr, &len);
  return ntohs(addr.sin_port);
}

static void prv_handle_rx(X86TcpThread *thread, X86TcpClient *client) {
  ssize_t read_len = read(client->fd, client->rx_buffer + client->rx_len,
                          X86_TCP_RX_BUFFER_LEN - client->rx_len);
  bool disconnected = false;
  if (read_len == 0) {
    // Disconnected - if there was data in the buffer, the packet was incomplete so it's invalid
    close(client->fd);
    memset(client, 0, sizeof(*client));
    client->fd = X86_TCP_INVALID_FD;
  } else if (read_len > 0) {
    client->rx_len += (size_t)read_len;

    uint32_t expected_len = 0;
    memcpy(&expected_len, client->rx_buffer, sizeof(expected_len));
    size_t header_len = sizeof(expected_len);
    if (client->rx_len < header_len && client->rx_len > 0) {
      // Invalid packet received - drop data
      client->rx_len = 0;
    }

    if (expected_len > 0 && (client->rx_len - header_len) == expected_len) {
      // Run handler - offset buffer to drop size information
      thread->handler(thread, client->fd, client->rx_buffer + header_len,
                      client->rx_len - header_len, thread->context);
      memset(client->rx_buffer, 0, sizeof(uint32_t));
      client->rx_len = 0;
    }
  }
}

static void *prv_server_thread(void *context) {
  X86TcpThread *thread = context;
  int server_fd = -1;
  if (!status_ok(prv_setup_socket(&server_fd))) {
    LOG_DEBUG("Socket setup failed\n");
    return NULL;
  }

  int port = prv_get_port(server_fd);
  LOG_DEBUG("Started RX server (%s) on port %d\n", thread->port_env_name, port);

  // Mutex unlocked when thread should exit
  while (pthread_mutex_trylock(&thread->keep_alive) != 0) {
    // Build new fd set - select will mangle it
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    int max_fd = server_fd;

    for (int i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
      if (thread->clients[i].fd != X86_TCP_INVALID_FD) {
        FD_SET(thread->clients[i].fd, &read_fds);
        max_fd = MAX(max_fd, thread->clients[i].fd);
      }
    }

    select(max_fd + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(server_fd, &read_fds)) {
      // Server read - new client
      struct sockaddr_in addr = { 0 };
      socklen_t addrlen = sizeof(addr);
      int new_socket = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
      if (new_socket < 0) {
        LOG_DEBUG("Failed to accept new client!\n");
      }
      LOG_DEBUG("New client %d from %s:%d connected!\n", new_socket, inet_ntoa(addr.sin_addr),
                ntohs(addr.sin_port));

      for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
        if (thread->clients[i].fd == X86_TCP_INVALID_FD) {
          thread->clients[i].fd = new_socket;
          break;
        }
      }
    }

    // Handle client reads - expect packets prefixed with a 32-bit unsigned integer representing
    // packet size
    for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
      X86TcpClient *client = &thread->clients[i];
      if (client->fd != X86_TCP_INVALID_FD && FD_ISSET(client->fd, &read_fds)) {
        prv_handle_rx(thread, client);
      }
    }
  }

  return NULL;
}

StatusCode x86_tcp_init(X86TcpThread *thread, char *port_env_name, X86TcpHandler handler, void *context) {
  memset(thread, 0, sizeof(*thread));
  thread->port_env_name = port_env_name;
  thread->handler = handler;
  thread->context = context;

  for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
    thread->clients[i].fd = X86_TCP_INVALID_FD;
  }

  pthread_mutex_init(&thread->client_lock, NULL);
  // Mutex is used as cancellation point for thread - locked = keep alive
  pthread_mutex_init(&thread->keep_alive, NULL);
  pthread_mutex_lock(&thread->keep_alive);

  pthread_create(&thread->thread, NULL, prv_server_thread, thread);

  return STATUS_CODE_OK;
}

StatusCode x86_tcp_broadcast(X86TcpThread *thread, const char *tx_data, size_t tx_len) {
  // TODO: does this need to be protected?
  pthread_mutex_lock(&thread->client_lock);
  for (size_t i = 0; i < X86_TCP_MAX_CLIENTS; i++) {
    if (thread->clients[i].fd != -1) {
      // TODO: write may not succeed or may only partially write - need to loop
      ssize_t write_len = write(thread->clients[i].fd, tx_data, tx_len);
    }
  }
  pthread_mutex_unlock(&thread->client_lock);

  return STATUS_CODE_OK;
}
