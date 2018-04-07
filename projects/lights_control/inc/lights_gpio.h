#pragma once
#include "event_queue.h"
#include "lights_config.h"
#include "status.h"

// initializes all the GPIO peripherals
StatusCode lights_gpio_init(LightsConfig *conf);

// sets the state of every peripheral related to a specific event
StatusCode lights_gpio_process_event(LightsConfig *conf, const Event *e);
