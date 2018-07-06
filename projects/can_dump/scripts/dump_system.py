#!/usr/bin/env python3
""" Dumps CAN information from SocketCAN/serial """
import argparse
from abc import abstractmethod
import binascii
import datetime
import logging
import os
import socket
import struct
import serial
import serial.tools.list_ports
from cobs import cobs

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

def data_battery_voltage_current(voltage, current):
    """Battery total voltage/current data format"""
    return '{:.4f}V {:.4f}A'.format(voltage / 10000, current / 1000000)

def data_dump(*args):
    """Generic data dump format"""
    return ' '.join(['{}'.format(arg) for arg in args])

# Name, struct format, data format 0, ...
MESSAGE_LOOKUP = {
    0: ('BPS Heartbeat', '<B', data_dump),
    1: ('Chaos Fault', '<B', data_dump),
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
    26: ('Charger state', '<B', data_dump),
    27: ('Charger relay', '<B', data_relay),
    32: ('Battery V/T', '<HHH', data_battery_vt),
    33: ('Battery Voltage/Current', '<ii', data_battery_voltage_current),
    35: ('Motor Bus Measurement', '<hhhh', data_dump),
    36: ('Motor Velocity', '<hh', data_dump),
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

    if msg_id in MESSAGE_LOOKUP:
        name, fmt, data_fn = MESSAGE_LOOKUP[msg_id]
        if fmt:
            try:
                unpacked_data = struct.unpack(fmt, data)
            except struct.error:
                print('Invalid {}'.format(msg_id))
                return
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
            print('Invalid device!')
            continue

class CanDataSource:
    def __init__(self, masked=None):
        self.masked = masked
        if masked is None:
            self.masked = []

    @abstractmethod
    def get_packet(self):
        pass

    def run(self):
        while True:
            # CAN ID, data, DLC
            can_id, data = self.get_packet()
            if can_id not in self.masked:
                parse_msg(can_id, data)

            logging.info('{},{:x},{}'.format(can_id, data, len(data)))

class SocketCanDataSource(CanDataSource):
    CAN_FRAME_FMT = "<IB3x8s"
    def __init__(self, masked):
        super().__init__(masked)
        self.sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
        self.sock.bind(("slcan0",))

    def get_packet(self):
        can_pkt = self.sock.recv(16)
        can_id, length, data = struct.unpack(self.CAN_FRAME_FMT, can_pkt)
        can_id &= socket.CAN_EFF_MASK
        data = data[:length]

        return can_id, data

class SerialCanDataSource(CanDataSource):
    def __init__(self, masked, device):
        super().__init__(masked)
        self.ser = serial.Serial(device, 115200)

    def get_packet(self):
        while True:
            encoded_line = self.readline()
            try:
                line = cobs.decode(encoded_line[:-1])
            except cobs.DecodeError:
                print('COBS decode error (len {})'.format(len(line)))
                continue

            if len(line) != 16:
                print('Invalid line (len {})'.format(len(line)))
                continue

            header = int.from_bytes(line[0:4], 'little')
            dlc = header >> 28 & 0xF

            can_id = int.from_bytes(line[4:8], 'little') & 0x7FF
            data = line[8:8 + dlc]

            yield can_id, data

    def readline(self, eol=b'\x00'):
        """Readline with arbitrary EOL delimiter

        Args:
            ser: Serial device to read
            eol: Bytes to use a EOL delimiter

        Returns:
            All data read from the device until the EOL delimiter was found.
        """
        leneol = len(eol)
        line = bytearray()
        while True:
            read_char = self.ser.read(1)
            if read_char:
                line += read_char
                if line[-leneol:] == eol:
                    break
            else:
                break
        return bytes(line)

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--log_dir', help='Directory for storing logs',
                        nargs='?', default='logs')
    parser.add_argument('-m', '--mask', help='Mask message ID from being parsed', action='append')
    parser.add_argument('device', help='Serial device or "slcan0"')
    args = parser.parse_args()

    log_file = '{}/system_can_{}.log'.format(args.log_dir, datetime.datetime.now())
    os.makedirs(os.path.dirname(log_file), exist_ok=True)
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s,%(message)s', filename=log_file)

    print('Masking IDs {}'.format(args.mask))
    if args.device == 'slcan0':
        datasource = SocketCanDataSource(masked=args.mask)
    else:
        datasource = SerialCanDataSource(masked=args.mask, device=args.device)

    datasource.run()

if __name__ == '__main__':
    main()
