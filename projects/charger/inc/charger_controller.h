#pragma once
// Module for controlling the charger
// Requires soft_timers, generic can and interrupts.
//
// This module periodically send and receives CAN Messages on both the Network and UART layers. CAN
// HW can be used to mock UART for testing.

#include <stdbool.h>

#include "can_interval.h"
#include "charger_can.h"
#include "generic_can.h"
#include "gpio.h"

typedef struct ChargerSettings {
  uint16_t max_voltage;
  uint16_t max_current;
  GenericCan *can;
  GenericCan *can_uart;
  GPIOAddress relay_control_pin;
} ChargerSettings;

// Initializes the charger controller. Expects |settings| to be fully populated.
StatusCode charger_controller_init(ChargerSettings *settings, ChargerCanStatus *status);

// Communicates with charger regarding what should be happening.
// CHARGER_STATE_START - Start charging.
// CHARGER_STATE_STOP  - Stop charging.
// CHARGER_STATE_OFF   - Charger is disconnected. Send one stop signal then turn off.
StatusCode charger_controller_set_state(ChargerCanState state);

// Checks that the internal |status| (memory provided in charger_controller_init) indicates a safe
// state.
bool charger_controller_is_safe(void);
