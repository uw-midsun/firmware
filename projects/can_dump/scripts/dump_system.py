#!/usr/bin/env python3
""" Dumps motor CAN information from SocketCAN """
import socket
import struct
import binascii

def parse_msg(can_id, data):
    """Parses and prints a system CAN message.

    Args:
        can_id: Raw standard CAN ID.
        data: Message data. Up to 8 bytes.

    Returns:
        None
    """

    # System CAN ID format:
    # [0:3] Source ID
    # [4] Message Type (ACK/DATA)
    # [5:10] Message ID
    source_id = can_id & 0xf
    msg_type = (can_id >> 4) & 0x1
    msg_id = (can_id >> 5) & 0x3f

    msg_type_name = 'ACK' if msg_type == 1 else 'data'

    if (msg_id == 32 and len(data) == 6):
        # Battery voltage/temperature
        vt_fmt = '<HHH'
        module, voltage, aux = struct.unpack(vt_fmt, data)
        print('C{}: {}mV aux {}mV'.format(module, voltage / 10, aux / 10))
    elif (msg_id == 0 and msg_type == 0):
        # BPS Heartbeat TX
        print('BPS Heartbeat: 0x{} from {}'.format(binascii.hexlify(data).decode('ascii'),
                                                   source_id))
    elif (msg_id == 0 and msg_type == 1):
        # BPS Heartbeat ACK
        print('BPS Hearbeat ACK from {}'.format(source_id))
    elif (msg_id == 2 or msg_id == 3):
        # Battery relay
        relay_name = 'main' if msg_id == 2 else 'slave'
        state = 'open' if data == 0x0 else 'close'
        print('Battery relay {}: {} ({}) from {}'.format(relay_name, state, msg_type_name,
                                                         source_id))
    else:
        # Unrecognized message
        print('{} from {} ({}): 0x{}'.format(msg_id, source_id, msg_type_name,
                                             binascii.hexlify(data).decode('ascii')))

def main():
    """Main entry point"""
    sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
    sock.bind(("slcan0",))
    fmt = "<IB3x8s"

    while True:
        can_pkt = sock.recv(16)
        can_id, length, data = struct.unpack(fmt, can_pkt)
        can_id &= socket.CAN_EFF_MASK
        data = data[:length]

        parse_msg(can_id, data)

if __name__ == '__main__':
    main()
