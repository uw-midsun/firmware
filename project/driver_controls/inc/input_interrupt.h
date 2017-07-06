#pragma once

#include "gpio.h"
#include "driver_state.h"

#define COAST_THRESHOLD 40
#define DRIVE_THRESHOLD 130

void input_callback(GPIOAddress *address, void *context);

