#include "x86_cmd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "x86_tcp.h"

#define X86_CMD_MAX_HANDLERS 16
#define X86_CMD_MAX_LEN 16

typedef struct X86CmdHandlerData {
  X86CmdHandler handler;
  void *context;
  const char *cmd;
} X86CmdHandlerData;

static X86CmdHandlerData s_handlers[X86_CMD_MAX_HANDLERS] = { 0 };
static size_t s_num_handlers = 0;

static void prv_parser(const char *str) {
  printf("Received command: %s\n", str);
  // only need to copy the first word - don't care about the rest
  char str_buf[X86_CMD_MAX_LEN] = { 0 };
  strncpy(str_buf, str, X86_CMD_MAX_LEN);
  char *cmd_word = strtok(str_buf, " ");

  for (size_t i = 0; i < s_num_handlers; i++) {
    if (strcmp(s_handlers[i].cmd, cmd_word) == 0) {
      s_handlers[i].handler(str, s_handlers[i].context);
      break;
    }
  }
}

static void prv_quit(const char *cmd_str, void *context) {
  printf("Command received - Quitting\n");
  exit(EXIT_SUCCESS);
}

void __attribute__((constructor)) x86_cmd_init(void) {
  x86_tcp_init(prv_parser);

  // Allow force quit on x86 - good for integration testing
  x86_cmd_register_handler("quit", prv_quit, NULL);
}

void x86_cmd_register_handler(const char *cmd, X86CmdHandler handler, void *context) {
  if (s_num_handlers < X86_CMD_MAX_HANDLERS) {
    s_handlers[s_num_handlers++] = (X86CmdHandlerData) {
      .handler = handler,
      .context = context,
      .cmd = cmd
    };
  }
}
