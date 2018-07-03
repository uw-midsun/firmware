#pragma once
// Module for interfacing with the charger proximity pin..
// Requires: event_queue, gpio, soft timers and ADC.
//
// The pin is expected to connect to the proximity pin on the charging port which indicates whether
// the device is connected. The expected values are as follows:
// | State        | Nominal Voltage (V) |
// |:------------:|:-------------------:|
// | Disconnected | 4.46                |
// | Unlatched    | 2.77                | Treated the same as disconnected for charging purposes.
// | Latched      | 1.53                |

#include "gpio.h"
#include "status.h"

#define CHARGER_PIN_POLL_PERIOD_MS 1500
// Threshold to consider a charger pin to be connected in mV. See the J1772 Charging standard for
// the proximity detection pin voltages. Above this threshold the charger is considered to be
// disconnected or unlatched so we shouldn't try to charge.
#define CHARGER_PIN_CONNECTED_THRESHOLD 1820

// Inits the charger proximity pin and configures a soft timer to periodically check the ADC.
// |address| is expected to persist.
StatusCode charger_pin_init(const GPIOAddress *address);
