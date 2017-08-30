#!/usr/bin/env python3
import serial
import struct
import ctypes

class CANIdBits(ctypes.LittleEndianStructure):
  _fields_ = [
    ('source_id', ctypes.c_uint16, 4),
    ('type', ctypes.c_uint16, 1),
    ('msg_id', ctypes.c_uint16, 6)
  ]

class CANId(ctypes.Union):
  _fields_ = [
    ('b', CANIdBits),
    ('as_u16', ctypes.c_uint16)
  ]

MSG_FORMAT = '<3cHQcc'

with serial.Serial('/dev/ttyACM0', 115200) as ser:
  while 1:
    line = ser.readline()
    try:
      raw_id, data = struct.unpack(MSG_FORMAT, line)[3:5]
      can_id = CANId()
      can_id.as_u16 = raw_id

      print('RX {0.b.msg_id} from {0.b.source_id} ({0.b.type}) - 0x{1:08x}'.format(can_id, data))
    except struct.error:
      pass
