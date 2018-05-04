#!/usr/bin/env python3
import socket
from subprocess import check_output

def get_socket(process, module):
  pid = int(check_output(['pidof', process]))
  return '\0{}/{}/{}'.format(pid, process, module)

socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
socketname = get_socket('test_x86_cmd_runner', 'cmd')
print(socketname)
socket.connect(socketname)
socket.send(b'test 4 b sd3d\n')
print(socket.recv(1024))
