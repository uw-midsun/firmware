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
  LightsEventGpioPeripheral peripheral_lookup[NUM_EE_LIGHT_TYPES];
  GPIOAddress rx_addr;
  GPIOAddress tx_addr;
  CANHwBitrate bitrate;
} LightsCanSettings;

// Initializes the lights_can module.
StatusCode lights_can_init(const LightsCanSettings *lights_can, LightsCanStorage *storage);
