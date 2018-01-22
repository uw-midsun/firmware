#pragma once

// Blocking debouncer for steering wheel interrupts
// GPIO and soft timers must be initialized

#include "gpio.h"
#include "status.h"

// Polls the input until it is steady and return the value
GPIOState debounce(GPIOAddress address);
