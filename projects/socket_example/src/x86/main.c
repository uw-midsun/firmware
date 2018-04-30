#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include "x86_socket.h"

static void prv_handler(X86SocketThread *thread, int client_fd, const char *rx_data, size_t rx_len,
                        void *context) {
  x86_socket_write(client_fd, "hello there\n", 12);
  printf("handling RX (%ld bytes)\n", rx_len);
  fwrite(rx_data, sizeof(char), rx_len, stdout);
}

int main(void) {
  X86SocketThread thread;
  x86_socket_init(&thread, "x86_test_port", prv_handler, NULL);

  while (true) {
  }

  return 0;
}
