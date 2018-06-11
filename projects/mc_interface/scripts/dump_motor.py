#!/usr/bin/env python3
import socket
import struct

def dump_msg(can_id, data, length):
  device_id = can_id >> 5
  msg_id = can_id & 0x1f

  is_motor = (device_id == 0x3 or device_id == 0x4)

  msg_lookup = {
    False: {
      # Driver Controls
      0x1: ('Drive', '<ff'),
      0x2: ('Power', '<fxxxx'),
      0x3: ('Reset', '<xxxxxxxx'),
    },
    True: {
      # Motor Controller
      0x0: ('ID', '<I4s'),
      0x1: ('Status', '<HHHH'),
      0x2: ('Bus Measurement', '<ff'),
      0x3: ('Velocity Measurement', '<ff'),
    }
  }

  name, fmt = msg_lookup[is_motor].get(msg_id, ('Unknown', '<ff'))
  friendly_type = 'MC' if is_motor else 'DC'

  print('Msg {} from {} 0x{:02x} - {}: {}'.format(msg_id, friendly_type, device_id, name, struct.unpack(fmt, data)))

def main():
  sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
  sock.bind(("slcan0",))
  fmt = "<IB3x8s"

  while True:
    can_pkt = sock.recv(16)
    can_id, length, data = struct.unpack(fmt, can_pkt)
    can_id &= socket.CAN_EFF_MASK
    data = data[:length]

    dump_msg(can_id, data, length)

main()
