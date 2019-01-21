import serial

data_read = True

with serial.Serial('/dev/ttyACM0', baudrate=115200) as ser:
	while data_read:
		data = ser.readline()

		if "START" in data:
			while True:
				data = ser.readline()
				data = data[44:]

				if "END" in data:
					data_read = False
					break
				print data