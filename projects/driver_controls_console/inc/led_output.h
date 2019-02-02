#pragma once
// Console LED Output Manager
// Processes events from the center console and changes the states of LEDs
// in the LED IO-Expander

#include "gpio_expander.h"
#include "input_event.h"
#include "status.h"

// Initialize all relevant pins in the LED IO-Expander
StatusCode led_output_init(GpioExpanderStorage *storage);

// Update the relevant LEDs from a raised Event
StatusCode led_output_process_event(Event *event);
