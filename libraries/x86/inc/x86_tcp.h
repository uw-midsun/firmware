#pragma once
#include <pthread.h>
#include <stddef.h>
#include "status.h"

#define X86_TCP_MAX_CLIENTS 5
#define X86_TCP_MAX_PENDING_CONNECTIONS 3
#define X86_TCP_RX_BUFFER_LEN 1024

typedef struct X86TcpClient {
  int fd;
  // First 4 bytes will be the expected packet length
  char rx_buffer[X86_TCP_RX_BUFFER_LEN];
  size_t rx_len;
} X86TcpClient;

// client_fd provided to allow reply to specific client
struct X86TcpThread;
typedef void (*X86TcpHandler)(struct X86TcpThread *thread, int client_fd, const char *rx_data, size_t rx_len, void *context);

typedef struct X86TcpThread {
  pthread_t thread;
  const char *port_env_name;
  X86TcpHandler handler;
  void *context;
  // Could probably use an object pool?
  X86TcpClient clients[X86_TCP_MAX_CLIENTS];
  pthread_mutex_t client_lock;
  pthread_mutex_t keep_alive;
} X86TcpThread;

// Initializes a new TCP server with a random port on localhost, using the provided buffer for TX
// Also creates an environment variable with the specified name and sets it to the server's port
// Note that threads are pretty large and should probably be allocated on the heap
StatusCode x86_tcp_init(X86TcpThread *thread, char *port_env_name, X86TcpHandler handler, void *context);

// Write to all connected clients
StatusCode x86_tcp_broadcast(X86TcpThread *thread, const char *tx_data, size_t tx_len);

// TODO: add destroy?
