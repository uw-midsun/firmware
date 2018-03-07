#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "can_interval.h"
#include "generic_can.h"
#include "soft_timer.h"

// Defined in datasheet.
typedef enum ChargerState {
  CHARGER_STATE_START = 0,
  CHARGER_STATE_STOP = 1,
  NUM_CHARGER_STATES,
} ChargerState;

// Also defined in datasheet.
typedef union ChargerStatus {
  uint8_t raw;
  struct {
    uint8_t hw_fault : 1;
    uint8_t over_temp : 1;
    uint8_t input_voltage : 1;
    uint8_t starting_state : 1;
    uint8_t comms_state : 1;
  };
} ChargerStatus;

typedef struct ChargerSettings {
  uint16_t max_voltage;
  uint16_t max_current;
  GenericCan *can;
  GenericCan *can_uart;
} ChargerSettings;

StatusCode charger_init(ChargerSettings *settings, ChargerStatus *status);

// Communicates with charger regarding what should be happening.
StatusCode charger_set_state(ChargerState state);
