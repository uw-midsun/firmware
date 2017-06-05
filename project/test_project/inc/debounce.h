#pragma once

#include "gpio.h"

// Hold time must be greater than sampling interval

#define SAMPLING_INTERVAL 100
#define HOLD_TIME 2000

void debounce(GPIOAddress* address, GPIOState* current_state);
