#pragma once

#include "gpio.h"

// Sets the value of current_state to the debounced state
void debounce(GPIOAddress *address, GPIOState *current_state);
