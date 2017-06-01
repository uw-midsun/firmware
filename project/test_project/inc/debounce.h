#pragma once

#include "gpio.h"

// Hold time must be greater than sampling interval

#define SAMPLING_INTERVAL 10
#define HOLD_TIME 200000

void debounce(GPIOAddress* address, GPIOState* current_state);
