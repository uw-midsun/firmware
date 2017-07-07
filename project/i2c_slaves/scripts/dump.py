#!/usr/bin/env python3
import serial

with serial.Serial('/dev/ttyACM0', 115200) as ser:
  for line in ser.readline():
    print(line)
