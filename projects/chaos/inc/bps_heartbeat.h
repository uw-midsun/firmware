#pragma once

#include "status.h"

#define BPS_HEARTBEAT_EXPECTED_PERIOD_MS 1000

// TODO(ELEC-105): Export enum for usage by Plutus
typedef enum BpsHeartbeatState {
  BPS_HEARTBEAT_STATE_OK = 0,
  BPS_HEARTBEAT_STATE_ERROR,
  NUM_BPS_HEARTBEAT_STATES,
} BpsHeartbeatState;

// Configures BPS heartbeat handler.
StatusCode bps_heartbeat_init(void);

// Manually start the BPS watchdog. Will start automatically on first successful and OK rx
// heartbeat otherwise.
StatusCode bps_heartbeat_start(void);
