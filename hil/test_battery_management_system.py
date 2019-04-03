import can
import cantools

import math

import unittest

import warnings


class TestBatteryMonitoringSystem(unittest.TestCase):
    """
    These examples are somewhat trivial, but this can be useful for boards any
    boards that rely heavily on receiving data over CAN, and currently have an
    observable state change over CAN. Ideally, we would move everything over
    CAN and broadcast the internal state of each board over CAN, which would
    allow us to test these by observing the state.

    Otherwise, we're just integration testing the ms-common drivers, which is
    limited in usefulness.

    This is the case for boards like Driver Controls (Steering), Driver
    Controls (Pedal), etc.
    """

    def setUp(self):
        # Disable deprecation warnings in cantools about python-can
        warnings.simplefilter("ignore", category=DeprecationWarning)

        # Either switch this to vcan0 (if using a virtual SocketCAN interface)
        # or can0
        self.can_bus = can.interface.Bus(channel="vcan0", bustype="socketcan")
        self.database = cantools.database.load_file("system_can.dbc")

    def test_receive_battery_aggregate_vc(self):
        tester = cantools.tester.Tester("PLUTUS", self.database, self.can_bus, None)

        tester.start()

        result = tester.expect(
            "BATTERY_AGGREGATE_VC", timeout=math.ceil((50 * 36) / 1000)
        )
        self.assertIsNotNone(result)
        self.assertEqual({"current": -123456, "voltage": 286331153}, result)

    def test_plutus_respond_to_powertrain_heartbeat(self):
        CAN_ACK_TIMEOUT = 25 / 1000
        tester = cantools.tester.Tester("PLUTUS", self.database, self.can_bus, None)

        tester.start()

        # Plutus should respond to Powertrain heartbeats
        tester.send("POWERTRAIN_HEARTBEAT")

        result = tester.expect(
            "POWERTRAIN_HEARTBEAT_ACK_FROM_PLUTUS", timeout=CAN_ACK_TIMEOUT, signals={}
        )
        self.assertIsNotNone(result)


if __name__ == "__main__":
    unittest.main()
