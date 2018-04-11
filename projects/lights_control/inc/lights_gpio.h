#pragma once
#include "event_queue.h"
#include "lights_gpio_config.h"
#include "status.h"

// This module abstracts the operation on gpio pins. It simplifies control of various peripherals
// by relating them to events. It needs to be initialized first to set up all the gpio pins.

// Initializes all the GPIO peripherals
StatusCode lights_gpio_init(LightsConfig *conf);

// Sets the state of every peripheral related to a specific event. Turns off the peripheral light
// if event's data is 0, and on if event's data is anything else.
StatusCode lights_gpio_process_event(LightsConfig *conf, const Event *e);
