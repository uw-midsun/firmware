#pragma once

#include "can.h"
#include "can_msg_defs.h"
#include "i2c.h"
#include "log.h"
#include "mcp3427.h"
#include "solar_master_event.h"

// Config provider for solar_master. Needs gpio to be initialized.

typedef enum {
  SOLAR_MASTER_CONFIG_BOARD_FRONT = 0,
  SOLAR_MASTER_CONFIG_BOARD_REAR,
  NUM_SOLAR_MASTER_CONFIG_BOARDS
} SolarMasterConfigBoard;

typedef struct SolarMasterConfig {
  I2CPort slave_i2c_port;
  I2CPort current_i2c_port;
  I2CSettings *slave_i2c_settings;
  I2CSettings *current_i2c_settings;
  Mcp3427Setting *slave_mcp3427_settings_base;
  CanSettings *can_settings;
  SolarMasterConfigBoard board;
} SolarMasterConfig;

// Reads GPIO to get the board type and adds it to the global config struct.
StatusCode solar_master_config_init(void);

// Returns a lazy static pointer to the global SolarMasterConfig struct.
SolarMasterConfig *solar_master_config_load(void);
