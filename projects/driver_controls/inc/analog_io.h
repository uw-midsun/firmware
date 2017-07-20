#pragma once

// Provides an interface to configure the GPIO pins for the driver control devices.
// This allows a cleaner device initialization than having to initialize each pin with
// different settings

#include "gpio.h"
#include "input_event.h"

// Initialize gpio pins and interrupts
void analog_io_init();
