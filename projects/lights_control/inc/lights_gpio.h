#pragma once
#include "event_queue.h"

#include "status.h"

typedef enum { LIGHTS_BOARD_FRONT, LIGHTS_BOARD_REAR, NUM_LIGHTS_BOARDS } LightsBoard;

// used for figuring out which board we're operating on is called before lights_gpio_init since
// lights_gpio_init needs to know which board it needs to initialize
StatusCode lights_gpio_get_lights_board(LightsBoard *board);

// initializes all the GPIO peripherals
StatusCode lights_gpio_init();

// sets the state of every peripheral related to a specific event
StatusCode lights_gpio_set(Event *e);
