#pragma once

#include "can.h"
#include "lights_events.h"

#define LIGHTS_CAN_NUM_RX_HANDLERS 1

typedef enum {
  LIGHTS_CAN_ACTION_SIGNAL_RIGHT = 0,
  LIGHTS_CAN_ACTION_SIGNAL_LEFT,
  LIGHTS_CAN_ACTION_SIGNAL_HAZARD,
  LIGHTS_CAN_ACTION_HORN,
  LIGHTS_CAN_ACTION_HIGH_BEAMS,
  LIGHTS_CAN_ACTION_LOW_BEAMS,
  LIGHTS_CAN_ACTION_DRL,
  LIGHTS_CAN_ACTION_BRAKES,
  LIGHTS_CAN_ACTION_STROBE,
  LIGHTS_CAN_ACTION_SYNC,
  NUM_LIGHTS_CAN_ACTIONS_ID
} LightsCanActionID;

// User uses this instance to provide storage for the module.
typedef struct LightsCanStorage {
  CANStorage can_storage;
  CANRxHandler rx_handlers[LIGHTS_CAN_NUM_RX_HANDLERS];
} LightsCanStorage;

// Contains all the configuration functions.
typedef struct LightsCanSettings {
  uint16_t device_id;
  bool loopback;
  LightsEvent event_lookup[NUM_LIGHTS_CAN_ACTIONS_ID];
  GPIOAddress rx_addr;
  GPIOAddress tx_addr;
} LightsCanSettings;

// Initializes the lights_can module.
StatusCode lights_can_init(const LightsCanSettings *lights_can, LightsCanStorage *storage);
