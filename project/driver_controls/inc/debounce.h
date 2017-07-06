#pragma once

#include "gpio.h"

// Hold times must be greater than or equal to the sampling interval

#define CHECK_MSEC    5     // Sampling interval in milliseconds
#define PRESS_MSEC    50    // Hold time for button presses
#define RELEASE_MSEC  50    // Hold time for button depresses

// Sets the value of current_state to the debounced state
void debounce(GPIOAddress *address, GPIOState *current_state);
