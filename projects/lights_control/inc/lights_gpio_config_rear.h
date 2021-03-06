#pragma once
// This module provides configuration for lights_gpio when operating on the rear board.

#include "lights_gpio.h"

// All the rear board outputs.
typedef enum {
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_STROBE = 0,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_BRAKE,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_BRAKE,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_BRAKE,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_BRAKE,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_CENTRE_BRAKE,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_TURN,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_TURN,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_TURN,
  LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_TURN,
  NUM_LIGHTS_GPIO_CONFIG_REAR_OUTPUTS,
} LightsGpioConfigRearOutputs;

// Loads configuration for rear board.
LightsGpio *lights_gpio_config_rear_load(void);
