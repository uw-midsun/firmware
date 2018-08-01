#!/usr/bin/env python3
""" Dumps motor CAN information from SocketCAN """
import socket
import struct

def dump_msg(can_id, data):
    """Parse and print WaveSculptor data.

    Args:
        can_id: CAN standard message ID.
        data: CAN message data. Should be 8 bytes.

    Returns:
        None
    """
    device_id = can_id >> 5
    msg_id = can_id & 0x1f

    is_motor = (device_id in [0x3, 0x4])

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
            0x4: ('Phase Current Measurement', '<ff'),
            0x5: ('Motor Voltage Vector Measurement', '<ff'),
            0x6: ('Motor Current Vector Measurement', '<ff'),
            0x7: ('Motor BackEMF Measurement / Prediction', '<ff'),
            0x8: ('15 & 1.65 Voltage Rail Measurement', '<ff'),
            0x9: ('2.5V & 1.2V Voltage Rail Measurement', '<ff'),
            0xA: ('Fan Speed Measurement', '<ff'),
            0xB: ('Sink & Motor Temperature Measurement', '<ff'),
            0xC: ('Air In & CPU Temperature Measurement', '<ff'),
            0xD: ('Air Out & Cap Temperature Measurement', '<ff'),
            0xE: ('Odometer & Bus AmpHours Measurement', '<ff'),
        }
    }

    name, fmt = msg_lookup[is_motor].get(msg_id, ('Unknown', '<ff'))
    friendly_type = 'MC' if is_motor else 'DC'

    unpacked = struct.unpack(fmt, data)
    try:
        data_str = '({:.4f}, {:.4f})'.format(*unpacked)
    except (ValueError, TypeError):
        data_str = unpacked
    print('Msg {} from {} 0x{:02x} - {}: {}'.format(msg_id, friendly_type, device_id,
                                                    name, data_str))

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

        if length == 8:
            dump_msg(can_id, data)
        else:
            print('RX id: 0x{:03x} data: 0x{:08x} (dlc {})'.format(can_id, data, length))

if __name__ == '__main__':
    main()
