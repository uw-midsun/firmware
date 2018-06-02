#!/usr/bin/env python3
import usb
import sys

if len(sys.argv) > 1:
  print('cmsis_dap_serial {}'.format(sys.argv[1]))
  exit()

devices = usb.core.find(find_all=True, idProduct=0xda42, idVendor=0x1209)

options = []

print('CMSIS-DAP devices:', file=sys.stderr)
for i, dev in enumerate(devices):
  serial = usb.util.get_string(dev, dev.iSerialNumber)
  print('{}: {}'.format(i, serial), file=sys.stderr)
  options.append(serial)

num_devices = sum(1 for i in devices)

if num_devices > 1:
  print('Select device: ', end='', file=sys.stderr)
  index = int(input())
  device = options[index]
  print('cmsis_dap_serial {}'.format(device))
