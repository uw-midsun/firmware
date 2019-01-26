#pragma once
#include "gpio.h"
#include "gpio_it.h"

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
