#pragma once

// Controls the Radio Group of LEDs consisting of the Drive, Neutral and
// Reverse LEDs.
//
// Requires the following to be initialized:
// - GPIO Expander

#include "fsm.h"
#include "gpio_expander.h"
#include "status.h"

// GPIO Expander pin mappings for LEDs being driven
typedef struct {
  GpioExpanderPin drive_pin;
  GpioExpanderPin neutral_pin;
  GpioExpanderPin reverse_pin;
} ButtonLedRadioSettings;

// Initialize the Radio Group FSM module
void button_led_radio_fsm_init(void);

// Create a new Radio Group FSM instance
StatusCode button_led_radio_fsm_create(Fsm *fsm, GpioExpanderStorage *storage,
                                       ButtonLedRadioSettings *settings, const char *fsm_name);
