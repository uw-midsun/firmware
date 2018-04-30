#!/usr/bin/env python3
import socket
import sys
from subprocess import check_output

def get_pids(process):
  return list(map(int, check_output(['pgrep',process]).split()))

pid = get_pids('socket_example')[0]

client = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
try:
  client.connect('\x00{}/socket_example/x86_test_port'.format(pid))
  client.send(b'hello world\n')
  client.send(b'this is a test\n')
  while True:
    print(client.recv(1024))
except socket.error:
  sys.exit(1)
