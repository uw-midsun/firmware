#!/usr/bin/env python3
"""Test CAN RX <-> UART script."""
import struct
import serial
import serial.tools.list_ports

# Packet format:
# 3c: CTX or CRX
# B: Header byte - extended flag, dlc
# I: u32 ID
# Q: u64 data
# c: Newline
PACKET_FMT = '3cBIQc'

def select_device():
    """User-provided serial device selector.

    Args:
        None

    Returns:
        The selected serial device as ListPortInfo.
    """
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
    """Parses the input line as an RX packet.

    Args:
        line: The input line. Should end with a newline.

    Returns:
        None
    """
    if len(line) != struct.calcsize(PACKET_FMT):
        return

    parsed_data = struct.unpack(PACKET_FMT, line)
    marker = ''.join([x.decode('ascii') for x in parsed_data[0:3]])
    header = parsed_data[3]
    extended = header & 0x1
    dlc = header >> 4 & 0xF
    can_id = parsed_data[4]
    data = parsed_data[5]
    print(marker, extended, dlc, can_id, data)

def parse_serial(device):
    """Prints transformed serial input.

    Valid RX packets will be decoded. All other input will be printed as is.

    Args:
        device: The input device location. (ex. /dev/ttyCMSIS)

    Returns:
        None
    """
    ser = serial.Serial(port=device, baudrate=115200)
    while True:
        line = ser.readline()
        if len(line) == 17:
            parse_line(line)
        else:
            print(line)

def main():
    """Main entry point"""
    dev = select_device()
    parse_serial(dev.device)

if __name__ == '__main__':
    main()
