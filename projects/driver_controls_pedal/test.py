import can
import cantools

import math

import unittest

import warnings


class TestBatteryMonitoringSystem(unittest.TestCase):
    def setUp(self):
        # Disable deprecation warnings in cantools about python-can
        warnings.simplefilter("ignore", category=DeprecationWarning)

         # Either switch this to vcan0 (if using a virtual SocketCAN interface)
        # or can0
        self.can_bus = can.interface.Bus(channel="vcan0", bustype="socketcan")
        self.database = cantools.database.load_file("system_can.dbc")

    def test_power_on(self):
        tester = cantools.tester.Tester("PEDAL", self.database, self.can_bus, None)
        tester.start()
        # TODO: Determine if we can use ENUM values
        tester.send('CENTER_CONSOLE_EVENT', {'event_id': 6, 'data': 0})

if __name__ == '__main__':
    unittest.main()
