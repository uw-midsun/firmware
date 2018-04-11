#pragma once

#include "event_queue.h"
#include "gpio.h"

// This module acts as storage for the lights_gpio module. It has to be initialized first as it
// needs to know the board we're operating on.

typedef enum {
  LIGHTS_CONFIG_BOARD_TYPE_FRONT = 0,
  LIGHTS_CONFIG_BOARD_TYPE_REAR,
  NUM_LIGHTS_CONFIG_BOARDS
} LightsConfigBoardType;

typedef struct LightsEventMapping {
  const EventID event_id;
  const uint16_t bitset;
} LightsEventMapping;

typedef struct LightsConfig {
  const LightsConfigBoardType board_type;
  const GPIOAddress *addresses;
  const uint8_t num_addresses;
  const LightsEventMapping *event_mappings;
  const uint8_t num_event_mappings;
} LightsConfig;

// user of this module will have to initialize it by passing in a fucntion that describes the
// board type.
typedef StatusCode (*LightsConfigBoardDescriptor)(LightsConfigBoardType *board_type);

// module init: must be called before attempting to load the config blob.
StatusCode lights_config_init(LightsConfigBoardDescriptor get_board_type);

// provides a config blob that includes all the information required by the lights_gpio module to
// set up gpio pins and addresses. It also includes mappings of events to gpio addresses which
// lights_gpio uses to process events.
LightsConfig *lights_config_load(void);
