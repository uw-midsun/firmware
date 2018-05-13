#pragma once
// This module loads the configuration for lights_gpio based on the board type.
// Needs to be initialized first.
//
// For board-specific configurations, refer to lights_gpio_config_front.h and
// lights_gpio_config_rear.h

#include "event_queue.h"
#include "gpio.h"
#include "lights_gpio.h"

typedef enum {
  LIGHTS_GPIO_CONFIG_BOARD_TYPE_FRONT = 0,  //
  LIGHTS_GPIO_CONFIG_BOARD_TYPE_REAR,       //
  NUM_LIGHTS_GPIO_CONFIG_BOARD_TYPES,       //
} LightsGpioConfigBoardType;

// Initializes the configuration based on the board type.
StatusCode lights_config_init(LightsGpioConfigBoardType board_type);

// Loads configuration for lights_gpio. Configuration includes mappings of peripherals to outputs.
LightsGpio *lights_config_load(void);
