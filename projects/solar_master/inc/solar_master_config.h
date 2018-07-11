#pragma once

// Config provider for solar_master. Needs gpio to be initialized.

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

// Reads GPIO to get the board type and adds it to the global config struct.
StatusCode solar_master_config_init(void);

// Returns a lazy static pointer to the global SolarMasterConfig struct.
SolarMasterConfig *solar_master_config_load(void);
