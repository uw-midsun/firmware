#pragma once
// This module loads the configuration for lights_gpio based on the board type.
// Needs to be initialized first.
//
// For board-specific configurations, refer to lights_gpio_config_front.h and
// lights_gpio_config_rear.h

#include "event_queue.h"
#include "gpio.h"
#include "lights_config.h"
#include "lights_gpio.h"

// Initializes the configuration based on the board type.
StatusCode lights_gpio_config_init(LightsConfigBoardType board_type);

// Loads configuration for lights_gpio. Configuration includes mappings of peripherals to outputs.
LightsGpio *lights_gpio_config_load(void);
