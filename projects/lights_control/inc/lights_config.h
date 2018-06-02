#pragma once

// Config for various modules. Probes the GPIO to figure out board type (front or back).
// Needs GPIO to be initialized.

typedef enum {
  LIGHTS_CONFIG_BOARD_TYPE_FRONT = 0,
  LIGHTS_CONFIG_BOARD_TYPE_REAR,
  NUM_LIGHTS_CONFIG_BOARD_TYPES
} LightsConfigBoardType;

LightsConfigBoardType lights_config_get_board_type(void);

