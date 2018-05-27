#pragma once

// This module is responsible for receiving various CAN messages and in return raising events to be
// processed by other modules.

#include "can.h"
#include "exported_enums.h"
#include "lights_events.h"

#define LIGHTS_CAN_NUM_RX_HANDLERS 5

// User uses this instance to provide storage for the module.
typedef struct LightsCanStorage {
  CANStorage can_storage;
  CANRxHandler rx_handlers[LIGHTS_CAN_NUM_RX_HANDLERS];
} LightsCanStorage;

// Contains all the configuration functions.
typedef struct LightsCanSettings {
  uint16_t device_id;
  bool loopback;
  uint8_t generic_light_lookup[NUM_LIGHTS_GENERIC_TYPES];
  GPIOAddress rx_addr;
  GPIOAddress tx_addr;
} LightsCanSettings;

// Initializes the lights_can module.
StatusCode lights_can_init(const LightsCanSettings *lights_can, LightsCanStorage *storage);
