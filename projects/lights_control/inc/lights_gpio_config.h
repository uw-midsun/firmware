#pragma once

#include "event_queue.h"
#include "gpio.h"
#include "lights_gpio.h"
#include "lights_gpio_config_front.h"
#include "lights_gpio_config_rear.h"

// This module loads the storage for lights_gpio based on the board type. For board-specific
// configurations, refer to lights_gpio_config_front.h and lights_gpio_config_rear.h

typedef enum {
  LIGHTS_GPIO_CONFIG_BOARD_TYPE_FRONT = 0,  //
  LIGHTS_GPIO_CONFIG_BOARD_TYPE_REAR,       //
  NUM_LIGHTS_GPIO_CONFIG_BOARD_TYPES,       //
} LightsGPIOConfigBoardType;

// Module initialization.
StatusCode lights_config_init(LightsGPIOConfigBoardType board_type);

// Provides storage for a lights_gpio module. It also includes mappings of events to gpio addresses
// which lights_gpio uses to process events.
LightsGPIO *lights_config_load(void);
