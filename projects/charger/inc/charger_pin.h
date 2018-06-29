#pragma once
// Module for interfacing with the charger proximity pin..
// Requires: event_queue, gpio, soft timers and ADC.

#include "gpio.h"
#include "status.h"

#define CHARGER_PIN_POLL_PERIOD_MS 1500

// Inits the charger proximity pin and configures a soft timer to periodically check the ADC.
// Address is expected to persist.
StatusCode charger_pin_init(const GPIOAddress *address);
