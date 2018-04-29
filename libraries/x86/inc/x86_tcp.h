#pragma once

typedef void (*X86TcpParser)(const char *str);

void x86_tcp_init(X86TcpParser parser);
