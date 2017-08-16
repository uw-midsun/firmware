#pragma once

// Driver for the TLV493D magnetic sensor
// I2C, interrupts, and soft timers must be initialized

#include "i2c.h"

typedef struct MagneticSensorReading {
  int16_t x;
  int16_t y;
  int16_t z;
} MagneticSensorReading;

// Start up the sensor with an initialized i2c port
StatusCode magnetic_sensor_init(I2CPort i2c_port);

// Read converted magnetic sensor values
StatusCode magnetic_sensor_read_data(I2CPort i2c_port, MagneticSensorReading *reading);
