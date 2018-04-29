#pragma once
#include <stddef.h>

typedef void (*X86CmdHandler)(const char *cmd_str, void *context);

void x86_cmd_init(void);

// Note that the handler will be called from another thread - code should be threadsafe
void x86_cmd_register_handler(const char *cmd, X86CmdHandler handler, void *context);
