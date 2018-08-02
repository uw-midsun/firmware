import argparse
import binascii
import csv

from can_message import CanMessage

def fix_hex(hex_value):
  # Takes a hex string in the form '0xaabbccdd' and
  # 1. Strips off the 0x
  # 2.
  return

def parse_data(file_name):
  log_reader = csv.reader(open(file_name, 'r'), delimiter=',')

  for row in log_reader:
    # data is logged in the format (timestamp, (can_id, data, len(data)))
    timestamp = 0
    can_id = int(row[1], 16)
    can_data = int(row[2], 16)

    data_len = int(row[3])

    msg = CanMessage(can_id, can_data.to_bytes(data_len, 'big'))

    msg.parse()


if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Parsing CAN log data')
  parser.add_argument('--file', required=True)

  args = parser.parse_args()

  parse_data(args.file)
