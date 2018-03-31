#pragma once
#include "event_queue.h"

#include "status.h"

typedef enum { LIGHTS_BOARD_FRONT = 0, LIGHTS_BOARD_REAR, NUM_LIGHTS_BOARDS } LightsBoard;

typedef enum {
  LIGHTS_GPIO_INIT_MODE_NORMAL = 0,
  LIGHTS_GPIO_INIT_MODE_TEST_FRONT,
  LIGHTS_GPIO_INIT_MODE_TEST_REAR,
  NUM_LIGHTS_GPIO_INIT_MODES
} LightsGPIOInitMode;

// initializes all the GPIO peripherals
StatusCode lights_gpio_init(LightsGPIOInitMode mode);

// used by other modules to get board type
StatusCode lights_gpio_get_lights_board(LightsBoard *board);

// sets the state of every peripheral related to a specific event
StatusCode lights_gpio_set(const Event *e);

// returns a pointer to the event mapping tables
uint16_t *test_lights_gpio_event_mappings(LightsBoard board);
