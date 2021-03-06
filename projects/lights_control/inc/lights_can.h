#pragma once
// This module is responsible for receiving CAN messages and raising events in return, or sending
// can messages.
#include "can.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "lights_events.h"

typedef enum {
  LIGHTS_CAN_EVENT_TYPE_GPIO = 0,
  LIGHTS_CAN_EVENT_TYPE_SIGNAL,
  LIGHTS_CAN_EVENT_TYPE_STROBE,
  NUM_LIGHTS_CAN_EVENT_TYPES
} LightsCanEventType;

// User uses this instance to provide storage for the module.
typedef struct LightsCanStorage {
  CanStorage can_storage;
} LightsCanStorage;

// Contains all the configuration functions.
typedef struct LightsCanSettings {
  LightsEvent event_type[NUM_EE_LIGHT_TYPES];
  uint16_t event_data_lookup[NUM_EE_LIGHT_TYPES];
} LightsCanSettings;

// Initializes the lights_can module.
StatusCode lights_can_init(LightsCanStorage *storage, const LightsCanSettings *lights_can,
                           const CanSettings *can_settings);

// Sends a sync message.
StatusCode lights_can_process_event(const Event *);
