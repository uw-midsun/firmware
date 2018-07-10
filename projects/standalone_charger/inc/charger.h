#pragma once
// Handles charger control/comms
// Requires soft timers, CAN, generic CAN to be initialized
#include "status.h"
#include "generic_can.h"
#include "charger_can.h"

#define CHARGER_BROADCAST_PERIOD_MS 1000

typedef void (*ChargerInfoCb)(uint16_t voltage, uint16_t current, ChargerCanStatus status, void *context);

typedef struct ChargerSettings {
  GenericCan *charger_can;
  ChargerInfoCb info_cb;
  void *context;
} ChargerSettings;

// |settings.charger_can| should be initialized
StatusCode charger_init(ChargerSettings *settings);

// In 0.1V, 0.1A
StatusCode charger_start(uint16_t voltage, uint16_t current);

// Stops charging
StatusCode charger_stop(void);
