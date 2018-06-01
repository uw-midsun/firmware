#pragma once
// This module is responsible for receiving CAN messages and raising events in return, or sending
// can messages.
#include "can.h"
#include "exported_enums.h"
#include "lights_events.h"

#define LIGHTS_CAN_NUM_RX_HANDLERS 4

typedef enum {
  LIGHTS_CAN_EVENT_TYPE_GPIO = 0,
  LIGHTS_CAN_EVENT_TYPE_SIGNAL,
  LIGHTS_CAN_EVENT_TYPE_STROBE,
  NUM_LIGHTS_CAN_EVENT_TYPES
} LightsCanEventType;

// User uses this instance to provide storage for the module.
typedef struct LightsCanStorage {
  CANStorage can_storage;
  CANRxHandler rx_handlers[LIGHTS_CAN_NUM_RX_HANDLERS];
} LightsCanStorage;

// Contains all the configuration functions.
typedef struct LightsCanSettings {
  uint16_t device_id;
  bool loopback;
  LightsEvent event_type[NUM_EE_LIGHT_TYPES];
  uint8_t event_data_lookup[NUM_EE_LIGHT_TYPES];
  GPIOAddress rx_addr;
  GPIOAddress tx_addr;
  CANHwBitrate bitrate;
} LightsCanSettings;

// Initializes the lights_can module.
StatusCode lights_can_init(LightsCanStorage *storage, const LightsCanSettings *lights_can);

// Sends a sync message.
StatusCode lights_can_transmit_sync(void);
