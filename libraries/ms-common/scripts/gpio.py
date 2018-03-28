import socket

TCP_IP = '127.0.0.1'
TCP_PORT = 4000
BUFFER_SIZE = 1024

# read to pin
def gpio_set_pin_state(port, pin, state):
  message = bytes([0x67, 0x70, 0x69, 0x6F, 0x20, port, pin, state, 0, 0])

  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.connect((TCP_IP, TCP_PORT))
  s.send(message)
  data = s.recv(BUFFER_SIZE)
  s.close()

  return data

# write to pin
def gpio_get_pin_state(port, pin, state):
  message = bytes([0x67, 0x70, 0x69, 0x6F, 0x20, port, pin, state, 1, 0])

  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.connect((TCP_IP, TCP_PORT))
  s.send(message)
  data = s.recv(BUFFER_SIZE)
  s.close()

  return data