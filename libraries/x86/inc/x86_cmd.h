#pragma once
// Launches socket to handle external commands
// Expects space-delimited ASCII commands
// Note that this only handles one command per packet
#include "x86_socket.h"

#define X86_CMD_SOCKET_NAME "cmd"
#define X86_CMD_MAX_HANDLERS 16
#define X86_CMD_MAX_ARGS 5

typedef void (*X86CmdHandlerFn)(int client_fd, const char *cmd, const char *args[], size_t num_args, void *context);

typedef struct X86CmdHandler {
  const char *cmd;
  X86CmdHandlerFn fn;
  void *context;
} X86CmdHandler;

typedef struct X86CmdThread {
  X86SocketThread socket;
  X86CmdHandler handlers[X86_CMD_MAX_HANDLERS];
  size_t num_handlers;
} X86CmdThread;

StatusCode x86_cmd_init(X86CmdThread *thread);

StatusCode x86_cmd_register_handler(X86CmdThread *thread, const char *cmd, X86CmdHandlerFn fn, void *context);

// Main command handler
X86CmdThread *x86_cmd_thread(void);
