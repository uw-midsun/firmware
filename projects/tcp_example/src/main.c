#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include "x86_tcp.h"

static void prv_handler(X86TcpThread *thread, int client_fd, const char *rx_data, size_t rx_len, void *context) {
  ssize_t write_len = write(client_fd, "hello there\n", 12);
  printf("handling RX (%ld bytes)\n", rx_len);
  fwrite(rx_data, sizeof(char), rx_len, stdout);
}

int main(void) {
  X86TcpThread thread;
  x86_tcp_init(&thread, "x86_test_port", prv_handler, NULL);

  while (true) {
  }

  return 0;
}