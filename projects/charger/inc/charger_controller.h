#pragma once
// Module for controlling the charger
// Requires soft_timers, generic can and interrupts.

#include <stdbool.h>
#include <stdint.h>

#include "can_interval.h"
#include "generic_can.h"

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

typedef struct ChargerControllerTxDataImpl {
  uint16_t max_voltage;
  uint16_t max_current;
  uint8_t charging;
} ChargerControllerTxDataImpl;

typedef union ChargerControllerTxData {
  uint64_t raw_data;
  ChargerControllerTxDataImpl data_impl;
} ChargerControllerTxData;

typedef struct ChargerControllerRxDataImpl {
  uint16_t voltage;
  uint16_t current;
  ChargerStatus status_flags;
} ChargerControllerRxDataImpl;

typedef union ChargerControllerRxData {
  uint64_t raw_data;
  ChargerControllerRxDataImpl data_impl;
} ChargerControllerRxData;

// Initializes the charger controller. Expects |settings| to be fully populated.
StatusCode charger_controller_init(ChargerSettings *settings, ChargerStatus *status);

// Communicates with charger regarding what should be happening.
StatusCode charger_controller_set_state(ChargerState state);

// Checks that |status| indicates a safe state.
bool charger_controller_is_safe(ChargerStatus status);
