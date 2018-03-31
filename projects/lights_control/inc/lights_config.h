#pragma once

#include "gpio.h"

typedef struct LightsConfig {
  const GPIOAddress *board_type_address;
  const GPIOAddress *addresses_front;
  const GPIOAddress *addresses_rear;
  uint8_t num_addresses_front;
  uint8_t num_addresses_rear;
  const GPIOSettings *gpio_settings_in;
  const GPIOSettings *gpio_settings_out;
} LightsConfig;

typedef enum {
  FRONT_LIGHT_HORN = 0,
  FRONT_LIGHT_HIGH_BEAMS_RIGHT,
  FRONT_LIGHT_HIGH_BEAMS_LEFT,
  FRONT_LIGHT_LOW_BEAMS_RIGHT,
  FRONT_LIGHT_LOW_BEAMS_LEFT,
  FRONT_LIGHT_DRL_RIGHT,
  FRONT_LIGHT_DRL_LEFT,
  FRONT_LIGHT_SIDE_LEFT_INDICATOR,
  FRONT_LIGHT_LEFT_TURN,
  FRONT_LIGHT_SIDE_RIGHT_INDICATOR,
  FRONT_LIGHT_RIGHT_TURN,
  NUM_FRONT_LIGHTS
} FrontLights;

typedef enum {
  REAR_LIGHT_STROBE = 0,
  REAR_LIGHT_RIGHT_BRAKE,
  REAR_LIGHT_RIGHT_OUTER_BRAKE,
  REAR_LIGHT_LEFT_BRAKE,
  REAR_LIGHT_LEFT_OUTER_BRAKE,
  REAR_LIGHT_CENTRE_BRAKE,
  REAR_LIGHT_LEFT_OUTER_TURN,
  REAR_LIGHT_LEFT_TURN,
  REAR_LIGHT_RIGHT_OUTER_TURN,
  REAR_LIGHT_RIGHT_TURN,
  NUM_REAR_LIGHTS,
} RearLights;

LightsConfig *lights_config_load(void);
