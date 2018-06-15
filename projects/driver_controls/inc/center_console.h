#pragma once
// Handles Center Console IO
// Requires GPIO expander, event queue to be initialized.
//
// Monitors MCP23008 over I2C and raises events on input state change.
// Also dedicates another MCP23008 to LED output for state notification.
//
// Raises INPUT_EVENT_CENTER_CONSOLE_* events with empty data fields.
// Assumes all inputs other than hazards are non-latching. The power button is required to be held
// down for the specified period before a power event will be raised.
#include <assert.h>
#include <stdint.h>
#include "gpio_expander.h"
#include "soft_timer.h"

// How long the power button must be held to count as a press
#define CENTER_CONSOLE_POWER_HOLD_MS 1

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

typedef struct CenterConsoleStorage {
  SoftTimerID hold_timer;
} CenterConsoleStorage;

// Sets up the expander as inputs to raise the associated events
StatusCode center_console_init(CenterConsoleStorage *storage, GpioExpanderStorage *expander);
