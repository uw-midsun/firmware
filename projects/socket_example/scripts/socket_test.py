#!/usr/bin/env python3
"""Sample script for interfacing with socket framework"""
import socket
import sys
from subprocess import check_output

def get_pids(process):
    """Returns a list of PIDs that match the given process name

    Args:
        process: string that is the process name to search

    Returns:
        List of integers representing the PIDs that match the process name
    """
    return list(map(int, check_output(['pgrep', process]).split()))

def main():
    """Main"""
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

if __name__ == '__main__':
    main()
