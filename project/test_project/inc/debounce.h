#pragma once

#include "gpio.h"

// Hold time must be greater than sampling rate 

#define SAMPLING_INTERVAL 1
#define HOLD_TIME_PRESSED 1500
#define HOLD_TIME_RELEASED 2000

void debounce(GPIOAddress* address, GPIOState key_pressed);
