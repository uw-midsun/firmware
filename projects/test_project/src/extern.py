import socket

TCP_IP = '127.0.0.1'
TCP_PORT = 4000
BUFFER_SIZE = 1024
MESSAGE = "gpio\n"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
s.send(MESSAGE.encode('utf-8'))
data = s.recv(BUFFER_SIZE)
s.close()

for i in range(0, len(data)):
  print(data[i])
print("received data: {}".format(''.str(data)))
