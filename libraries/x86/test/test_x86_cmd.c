#include <stdbool.h>
#include <string.h>
#include "log.h"
#include "unity.h"
#include "x86_cmd.h"

static const char s_cmd[30];
static size_t s_num_args;

static void prv_handler(int client_fd, const char *cmd, const char *args[], size_t num_args,
                        void *context) {
  bool *received = context;
  LOG_DEBUG("Handling cmd %s (%d args) from %d\n", cmd, num_args, client_fd);
  for (size_t i = 0; i < num_args; i++) {
    LOG_DEBUG("Arg %d: %s\n", i, args[i]);
  }

  const char *msg = "Response\n";
  x86_socket_write(client_fd, msg, strlen(msg));

  strncpy(s_cmd, cmd, sizeof(s_cmd));
  s_num_args = num_args;
  *received = true;
}

void setup_test(void) {
  memset(s_cmd, 0, sizeof(s_cmd));
  s_num_args = 0;
}

void teardown_test(void) {}

void test_x86_cmd_client(void) {
  volatile bool received = false;
  x86_cmd_register_handler("test", prv_handler, &received);

  // Delay so we hopefully start the client after the RX server thread has actually initialized.
  LOG_DEBUG("Hopefully delaying a bit for the RX server to start\n");
  for (volatile uint32_t i = 0; i < 100000000; i++) {
  }
  int client_fd = test_x86_socket_client_init(X86_CMD_SOCKET_NAME);

  const char *cmd = "test a b c d";
  LOG_DEBUG("Sending command: \"%s\"\n", cmd);
  x86_socket_write(client_fd, cmd, strlen(cmd));

  while (!received) {
  }

  TEST_ASSERT_EQUAL_STRING("test", s_cmd);
  TEST_ASSERT_EQUAL(4, s_num_args);
}
