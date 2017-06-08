#pragma once

#include "gpio.h"

// Hold time must be greater than sampling interval

#define SAMPLING_INTERVAL 100   // Number of reads inbetween sampling
#define HOLD_TIME 200000          // Number of samples that the signal needs to remain steady for

// Return the debounced state of the pressed button
void debounce(GPIOAddress* address, GPIOState* current_state);
