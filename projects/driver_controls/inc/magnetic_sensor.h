#pragma once

// Driver for the TLV493D magnetic sensor
// I2C must be enabled

#include "i2c.h"

typedef enum {
  MAGNETIC_SENSOR_PROBE_BX = 0,
  MAGNETIC_SENSOR_PROBE_BY,
  MAGNETIC_SENSOR_PROBE_BZ,
  NUM_MAGNETIC_SENSOR_PROBES
} MagneticSensorProbe;

StatusCode magnetic_sensor_init(I2CPort i2c_port);

StatusCode magnetic_sensor_read_data(I2CPort i2c_port, int16_t *reading);
