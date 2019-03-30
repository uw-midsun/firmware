# Usage

```bash
# Install requirements
pip install -r requirements.txt

# Load kernel modules
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan

# Bring up SocketCAN interface 
# If you're using a PEAK PCAN-Dongle:
sudo ip link set can0 up type can bitrate 500000
# If you're using a slcan device
sudo slcand -o -c -s6 /dev/ttyUSB0 slcan0
sudo ifconfig slcan0 up

# Run the tests, looking for files that match the 'test_*.py' pattern
python -m unittest discover -s . -p 'test_*.py'

# Write new tests
vim test_some_new_board.py

# Auto format python
black .
```

# Notes

We probably want to move the DBC up 1 directory, but this is just a demo.
