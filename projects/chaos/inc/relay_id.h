#pragma once

// TODO(ELEC-105): Export this to a shared library for use by boards with Relays.
typedef enum {
  RELAY_ID_SOLAR_MASTER_FRONT = 0,
  RELAY_ID_SOLAR_MASTER_REAR,
  RELAY_ID_BATTERY_MAIN,
  RELAY_ID_BATTERY_SLAVE,
  RELAY_ID_MOTORS,
  NUM_RELAY_IDS,
} RelayId;

typedef enum {
  RELAY_STATE_OPEN = 0,
  RELAY_STATE_CLOSE = 1,
} RelayState;
