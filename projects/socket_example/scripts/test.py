#!/usr/bin/env python3
import socket
import sys

client = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
try:
  client.connect("\x000/socket_example/x86_test_port")
  client.send(b"hello world\n")
  client.send(b"this is a test\n")
  while True:
    print(client.recv(1024))
except socket.error:
  sys.exit(1)
