#!/usr/bin/env python3
""" Dumps motor CAN information from SocketCAN """
from cobs import cobs
import socket
import struct
import serial
import binascii
import serial.tools.list_ports

DATA_POWER_STATE = ['idle', 'charge', 'drive']
LIGHTS_ID_NAME = [
    'High beams', 'Low beams', 'DRL', 'Brakes',
    'Left Turn', 'Right Turn', 'Hazards', 'BPS Strobe'
]

def data_relay(state):
    """Relay state data format"""
    return 'close' if state else 'open'

def data_power_state(state):
    """Power state data format"""
    return DATA_POWER_STATE[state]

def data_lights_state(light_id, state):
    """Lights state data format"""
    id_name = LIGHTS_ID_NAME[light_id]
    state_name = 'on' if state else 'off'
    return '{}: {}'.format(id_name, state_name)

def data_battery_vt(module, voltage, temperature):
    """Battery V/T data format"""
    return 'C{}: {:.1f}mV aux {:.1f}mV'.format(module, voltage / 10, temperature / 10)

def data_battery_current(raw_lsbs):
    """Battery Current data format"""
    # Note that this is just an estimation - it should be updated once we have current calibration
    return '~{}uA'.format(raw_lsbs / 2)

def data_dump(*args):
    """Generic data dump format"""
    return ' '.join(['{}'.format(arg) for arg in args])

# Name, struct format, data format 0, ...
MESSAGE_LOOKUP = {
    0: ('BPS Heartbeat', '<B', data_dump),
    1: ('Chaos Fault', '', data_dump),
    2: ('Battery relay (Main)', '<B', data_relay),
    3: ('Battery relay (Slave)', '<B', data_relay),
    4: ('Motor relay', '<B', data_relay),
    5: ('Solar relay (Rear)', '<B', data_relay),
    6: ('Solar relay (Front)', '<B', data_relay),
    7: ('Power state', '<B', data_power_state),
    8: ('Chaos Heartbeat', '', data_dump),
    18: ('Drive Output', '<hhhh', data_dump),
    19: ('Cruise Target', '<B', data_dump),
    23: ('Lights Sync', '', data_dump),
    24: ('Lights State', '<BB', data_lights_state),
    32: ('Battery V/T', '<HHH', data_battery_vt),
    33: ('Battery Current', '<i', data_battery_current),
    36: ('Motor Velocity', '<ii', data_dump),
    43: ('Aux & DC/DC V/C', '<HHHH', data_dump),
}

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

    msg_type_name = 'ACK' if msg_type == 1 else 'DATA'

    masked = []

    if msg_id in masked:
        pass
    elif msg_id in MESSAGE_LOOKUP:
        name, fmt, data_fn = MESSAGE_LOOKUP[msg_id]
        if fmt:
            unpacked_data = struct.unpack(fmt, data)
        else:
            unpacked_data = []

        if msg_type_name == 'ACK':
            print('{} ACK from {}'.format(name, source_id))
        else:
            print('{}: {}'.format(name, data_fn(*unpacked_data)))
    else:
        # Unrecognized message
        print('{} from {} ({}): 0x{}'.format(msg_id, source_id, msg_type_name,
                                             binascii.hexlify(data).decode('ascii')))

def main_serial():
    def readline(a_serial, eol=b'\x00'):
        leneol = len(eol)
        line = bytearray()
        while True:
            c = a_serial.read(1)
            if c:
                line += c
                if line[-leneol:] == eol:
                    break
            else:
                break
        return bytes(line)

    ports = [comport.device for comport in serial.tools.list_ports.grep('usbserial')]

    count = 0
    with serial.Serial(ports[0], 115200) as ser:
        while True:
            encoded_line = readline(ser)
            try:
                line = cobs.decode(encoded_line[:-1])
            except cobs.DecodeError:
                continue

            if len(line) != 16:
                print('Invalid line (len {})'.format(len(line)))
                continue
            header = int.from_bytes(line[0:4], 'little')
            marker = header & 0xFFF
            extended = header >> 24 & 0x1
            rtr = header >> 25 & 0x1
            dlc = header >> 28 & 0xF

            can_id = int.from_bytes(line[4:8], 'little') & 0x7FF
            data = line[8:8 + dlc]

            parse_msg(can_id, data)

def main_socketcan():
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
    main_serial()
