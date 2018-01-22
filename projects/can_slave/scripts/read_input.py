#!/usr/bin/env python3
import serial
import serial.tools.list_ports
import struct

PACKET_FMT = '3cBIQc'

def select_device():
  while True:
    print('Pick the serial device:')
    ports = serial.tools.list_ports.comports()
    for i, port in enumerate(ports):
      print('{}: {}'.format(i, port))

    try:
      chosen_port = ports[int(input())]
      print('Selected {}'.format(chosen_port))
      return chosen_port
    except IndexError:
      print('Invalid index!')
      continue

def parse_line(line):
  if (len(line) != 17):
    return

  parsed_data = struct.unpack(PACKET_FMT, line)
  marker = ''.join([x.decode('ascii') for x in parsed_data[0:3]])
  header = parsed_data[3]
  extended = header & 0x1;
  dlc = header >> 4 & 0xF;
  can_id = parsed_data[4]
  data = parsed_data[5]

  print(marker, extended, dlc, can_id, data)

def parse_serial(port):
  ser = serial.Serial(port=port.device, baudrate=115200)
  while True:
    line = ser.readline()
    if (len(line) == 17):
      parse_line(line)
    else:
      print(line)

if __name__ == '__main__':
  port = select_device()
  parse_serial(port)
