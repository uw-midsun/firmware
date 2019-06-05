#pragma once

#include "button_led_radio_fsm.h"
#include "event_queue.h"
#include "gpio_expander.h"
#include "status.h"

// Initializes the LED button FSMs
StatusCode button_led_radio_init(GpioExpanderStorage *storage, ButtonLedRadioSettings *settings);

// Updates the LED buttons based on an event
bool button_led_radio_process_event(const Event *e);
