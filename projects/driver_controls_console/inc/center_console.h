#pragma once
// Handles Center Console IO
// Requires GPIO, GPIO Interrupts, event queue, soft timer to be initialized

#include "gpio.h"
#include "gpio_it.h"

#define CENTER_CONSOLE_POLL_PERIOD_MS 1000

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
} CenterConsoleStorage;

// Sets up the expander as inputs to raise the associated events
StatusCode center_console_init(CenterConsoleStorage *storage, GpioExpanderStorage *expander);
