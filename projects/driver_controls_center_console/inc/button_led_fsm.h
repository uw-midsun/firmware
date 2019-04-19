#pragma once

// TODO: Finish implementing this
// This module uses the MCP23008 IO/Expander and controls the LED buttons

#include "fsm.h"
#include "gpio_expander.h"
#include "status.h"

typedef enum {
  CENTER_CONSOLE_BUTTON_LED_BPS = 0,
  CENTER_CONSOLE_BUTTON_LED_PWR,
  CENTER_CONSOLE_BUTTON_LED_REVERSE,
  CENTER_CONSOLE_BUTTON_LED_NEUTRAL,
  CENTER_CONSOLE_BUTTON_LED_DRIVE,
  CENTER_CONSOLE_BUTTON_LED_DRL,
  CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS,
  CENTER_CONSOLE_BUTTON_LED_HAZARDS,
  NUM_CENTER_CONSOLE_BUTTON_LEDS,
} CenterConsoleButtonLed;

// Initialize the button_led_fsm module.
void button_led_fsm_init(void);

// Create a new button FSM instance for a CenterConsoleButtonLed.
StatusCode button_led_fsm_create(Fsm *fsm, GpioExpanderStorage *expander_storage,
                                 CenterConsoleButtonLed button_id, GpioExpanderPin pin,
                                 const char *fsm_name);
