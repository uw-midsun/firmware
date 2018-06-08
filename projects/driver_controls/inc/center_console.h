#pragma once
// Handles Center Console IO
// Requires GPIO expander, event queue to be initialized.
//
// Monitors MCP23008 over I2C and raises events on input state change.
// Also dedicates another MCP23008 to LED output for state notification.
//
// Raises INPUT_EVENT_CENTER_CONSOLE_* events with empty data fields.
#include <assert.h>
#include <stdint.h>
#include "gpio_expander.h"

#define CENTER_CONSOLE_NUM_INPUTS 7
static_assert(CENTER_CONSOLE_NUM_INPUTS <= NUM_GPIO_EXPANDER_PINS,
              "Center console inputs larger than number of MCP23008 pins!");
