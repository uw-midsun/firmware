#!/usr/bin/env python3
""" Dumps motor CAN information from SocketCAN """
import socket
import struct
import binascii

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

        source_id = can_id & 0xf
        msg_type = (can_id >> 4) & 0x1
        msg_id = (can_id >> 5) & 0x3f

        # print('RX {} from {} (ack {}) data: 0x{} (dlc {})'.format(msg_id, source_id, msg_type, binascii.hexlify(data), length))
        msg_type_name = 'ACK' if msg_type == 1 else 'data'
        if (msg_id == 32 and length == 6):
            vt_fmt = '<HHH'
            module, voltage, temp = struct.unpack(vt_fmt, data)
            # print('C{}: {}mV'.format(module, voltage / 10))
        elif (msg_id == 0 and msg_type == 0):
            print('BPS Heartbeat: 0x{} from {}'.format(binascii.hexlify(data), source_id))
        elif (msg_id == 0 and msg_type == 1):
            print('BPS Hearbeat ACK from {}'.format(source_id))
        elif (msg_id == 2 or msg_id == 3):
            relay_name = 'main' if msg_id == 2 else 'slave'
            print('Battery relay {}: {} ({}) from {}'.format(relay_name, data, msg_type_name, source_id))

if __name__ == '__main__':
    main()
