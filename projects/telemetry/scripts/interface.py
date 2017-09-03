#!/usr/bin/env python3
"""
Test RX interface for telemetry's semi-binary protocol
"""
import struct
import serial

# Struct format code:
# * Little endian
# * 3 characters (RX[)
# * uint16_t (id)
# * uint8_t (dlc)
# * uint64_t (data)
# * 2 characters (]\n)
# See https://docs.python.org/3/library/struct.html#format-characters
MSG_FORMAT = '<3cHBQ2c'

def extract_can_id(raw_id):
    """ Parses a raw CAN ID for our message format.

    See can_msg.h for the full message format.

    Args:
        raw_id: 11-bit integer

    Returns:
        Dictionary: keys will correspond to the CAN ID fields.
    """
    id_fmt = [('source_id', 4), ('type', 1), ('msg_id', 6)]

    ret = {}

    pos = 0
    for name, size in id_fmt:
        mask = ((1 << size) - 1) << pos
        ret[name] = (mask & raw_id) >> pos
        pos += size

    return ret

def main():
    """ Main event loop """
    with serial.Serial('/dev/ttyACM1', 115200) as ser:
        while 1:
            line = ser.readline()
            try:
                raw_id, dlc, data = struct.unpack(MSG_FORMAT, line)[3:6]
                can_id = extract_can_id(raw_id)

                # pylint: disable=invalid-format-index
                print('RX {0[msg_id]} from {0[source_id]} ({0[type]})'
                      ' - {1} bytes: 0x{2:08x}'.format(can_id, dlc, data))
            except struct.error:
                print(line)
            except KeyboardInterrupt:
                break



if __name__ == '__main__':
    main()
