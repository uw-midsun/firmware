#pragma once

#include "event_queue.h"
#include "gpio_expander.h"

// Initializes the LED button FSMs
void button_led_init(GpioExpanderStorage *storage, GpioExpanderPin *pins);

// Updates the LED buttons based on an event
bool button_led_process_event(const Event *e);
