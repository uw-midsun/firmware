#pragma once
#include "event_queue.h"
#include "lights_config.h"
#include "status.h"

typedef enum {
  LIGHTS_GPIO_INIT_MODE_NORMAL = 0,
  LIGHTS_GPIO_INIT_MODE_TEST_FRONT,
  LIGHTS_GPIO_INIT_MODE_TEST_REAR,
  NUM_LIGHTS_GPIO_INIT_MODES
} LightsGPIOInitMode;

// initializes all the GPIO peripherals
StatusCode lights_gpio_init(LightsConfig *conf);

// sets the state of every peripheral related to a specific event
StatusCode lights_gpio_process_event(const Event *e, LightsConfig *conf);
