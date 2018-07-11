#pragma once

#include "i2c.h"

typedef enum {
  SOLAR_MASTER_CONFIG_BOARD_FRONT = 0,
  SOLAR_MASTER_CONFIG_BOARD_REAR,
  NUM_SOLAR_MASTER_CONFIG_BOARDS
} SolarMasterConfigBoard;

typedef struct SolarMasterConfig {
  I2CPort i2c_port;
  SolarMasterConfigBoard board;
} SolarMasterConfig;

StatusCode solar_master_config_init(void);

SolarMasterConfig *solar_master_config_load(void);
