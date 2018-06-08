#pragma once
// Handles Center Console IO
// Requires GPIO expander, event queue to be initialized.
//
// Monitors MCP23008 over I2C and raises events on input state change.
// Also dedicates another MCP23008 to LED output for state notification.
//
// Raises INPUT_EVENT_CENTER_CONSOLE_* events with empty data fields.
// Assumes all inputs other than hazards are non-latching.
#include <assert.h>
#include <stdint.h>
#include "gpio_expander.h"

typedef enum {
  CENTER_CONSOLE_INPUT_POWER = 0,
  CENTER_CONSOLE_INPUT_DRIVE,
  CENTER_CONSOLE_INPUT_NEUTRAL,
  CENTER_CONSOLE_INPUT_REVERSE,
  CENTER_CONSOLE_INPUT_DRL,
  CENTER_CONSOLE_INPUT_LOWBEAM,
  CENTER_CONSOLE_INPUT_HAZARDS,
  NUM_CENTER_CONSOLE_INPUTS
} CenterConsoleInput;

// Sets up the expander as inputs to raise the associated events
StatusCode center_console_init(GpioExpanderStorage *expander);
