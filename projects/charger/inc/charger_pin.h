#pragma once
// Module for interfacing with the charger state pin.
// Requires: event_queue, interrupts, gpio and gpio_it to be initialized

#include "gpio.h"
#include "status.h"

// Inits the charger pin and configures an interrupt.
StatusCode init_charger_pin(const GPIOAddress *address);
