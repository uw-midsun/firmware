#pragma once

#include "i2c.h"

typedef enum {
  SOLAR_SENSE_CONFIG_BOARD_FRONT = 0,
  SOLAR_SENSE_CONFIG_BOARD_REAR,
  NUM_SOLAR_SENSE_CONFIG_BOARDS
} SolarSenseConfigBoard;

typedef struct SolarSenseConfig {
  I2CPort i2c_port;
  SolarSenseConfigBoard board;
} SolarSenseConfig;


StatusCode solar_sense_config_init(void);

SolarSenseConfig *solar_sense_config_load(void);



