#pragma once

// Module provider configurations for lights_can. Needs to be initialized first.

#include "can.h"
#include "gpio.h"

#include "lights_can.h"

// Used for determining the board type.
typedef enum {
  LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT = 0,
  LIGHTS_CAN_CONFIG_BOARD_TYPE_REAR,
  NUM_LIGHTS_CAN_CONFIG_BOARD_TYPES
} LightsCanConfigBoardType;

// Initializes configuration based on board type.
void lights_can_config_init(LightsCanConfigBoardType can_board);

// Loads configuration blob for lights_can.
const LightsCanSettings *lights_can_config_load(void);
