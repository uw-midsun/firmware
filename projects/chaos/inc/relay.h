#pragma once

#include <stdbool.h>

#include "event_queue.h"
#include "gpio.h"
#include "relay_id.h"
#include "status.h"

typedef struct RelaySettings {
  GPIOAddress battery_main_power_pin;
  GPIOAddress battery_slave_power_pin;
  GPIOAddress motor_power_pin;
  GPIOAddress solar_front_power_pin;
  GPIOAddress solar_rear_power_pin;
  bool loopback;
} RelaySettings;

// Initializes the relay FSMs.
void relay_init(const RelaySettings *settings);

// Updates the relays based on an event.
bool relay_process_event(const Event *e);
