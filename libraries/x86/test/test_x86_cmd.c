#include "unity.h"
#include <string.h>
#include <stdbool.h>
#include "log.h"
#include "x86_cmd.h"

static void prv_handler(int client_fd, const char *cmd, const char *args[], size_t num_args, void *context) {
  LOG_DEBUG("Handling cmd %s (%d args) from %d\n", cmd, num_args, client_fd);
  for (size_t i = 0; i < num_args; i++) {
    LOG_DEBUG("Arg %d: %s\n", i, args[i]);
  }

  const char *msg = "Response\n";
  x86_socket_write(client_fd, msg, strlen(msg));
}

void setup_test(void) {

}

void teardown_test(void) {

}

void test_x86_cmd_client(void) {
  X86CmdThread thread;
  x86_cmd_init(&thread);
  x86_cmd_register_handler(&thread, "test", prv_handler, NULL);

  while (true);
}
