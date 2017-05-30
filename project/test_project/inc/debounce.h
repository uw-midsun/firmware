#pragma once

#include "gpio.h"

// Hold time must be greater than sampling interval

#define SAMPLING_INTERVAL 1
#define HOLD_TIME 2000

void debounce(GPIOAddress* address);
