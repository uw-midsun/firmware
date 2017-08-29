#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>

void  __attribute__((constructor)) x86_cmd_init(void) {
  printf("hello\n");
}
