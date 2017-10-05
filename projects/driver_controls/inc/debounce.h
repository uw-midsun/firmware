#pragma once

// GPIO and soft timers must be initialized

#include "gpio.h"
#include "status.h"

GPIOState debounce(GPIOAddress address, GPIOState* );