#pragma once
// Handles Center Console IO
// Requires GPIO expander, event queue to be initialized.
//
// Monitors MCP23008 over I2C and raises events on input state change.
// Also dedicates another MCP23008 to LED output for state notification.
//
// Raises INPUT_EVENT_CENTER_CONSOLE_* events with empty data fields.
// Assumes all inputs other than hazards are non-latching.
//
// Processes:
// - INPUT_EVENT_DIRECTION_STATE_*
// - INPUT_EVENT_POWER_STATE_*
// - INPUT_EVENT_HEADLIGHT_STATE_*
// - INPUT_EVENT_HAZARDS_STATE_*
// and updates the center console LEDs appropriately. Assumes LEDs are active-low.
#include <assert.h>
#include <stdint.h>
#include "gpio_expander.h"
#include "soft_timer.h"

typedef enum {
  CENTER_CONSOLE_INPUT_POWER = 0,
  CENTER_CONSOLE_INPUT_DRIVE,
  CENTER_CONSOLE_INPUT_NEUTRAL,
  CENTER_CONSOLE_INPUT_REVERSE,
  CENTER_CONSOLE_INPUT_DRL,
  CENTER_CONSOLE_INPUT_LOWBEAM,
  CENTER_CONSOLE_INPUT_HAZARDS,
  NUM_CENTER_CONSOLE_INPUTS,
} CenterConsoleInput;

typedef enum {
  CENTER_CONSOLE_OUTPUT_POWER_LED = 0,
  CENTER_CONSOLE_OUTPUT_DRIVE_LED,
  CENTER_CONSOLE_OUTPUT_NEUTRAL_LED,
  CENTER_CONSOLE_OUTPUT_REVERSE_LED,
  CENTER_CONSOLE_OUTPUT_DRL_LED,
  CENTER_CONSOLE_OUTPUT_LOWBEAM_LED,
  CENTER_CONSOLE_OUTPUT_HAZARDS_LED,
  CENTER_CONSOLE_OUTPUT_BPS_FAULT_LED,
  NUM_CENTER_CONSOLE_OUTPUTS,
} CenterConsoleOutput;

typedef struct CenterConsoleStorage {
  GpioExpanderStorage *output_expander;
} CenterConsoleStorage;

// Sets up the expander as inputs to raise the associated events
StatusCode center_console_init(CenterConsoleStorage *storage, GpioExpanderStorage *input_expander,
                               GpioExpanderStorage *output_expander);

// Processes center console-related events and changes the IO expander output states.
bool center_console_process_event(CenterConsoleStorage *storage, const Event *e);
