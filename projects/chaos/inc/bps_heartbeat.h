#pragma once
// Module for receiving periodic heartbeats from the BPS. Power distribution boots the BPS board as
// one of its first actions and expects heartbeats immediately thereafter. If the watchdog ever
// times out then a major failure has occurred. The intent is that 1-2 failures are forgiven but any
// more immediately signal an Emergency state. The heartbeat watchdog should be restarted if the car
// recovers from Emergency into Idle but if the BPS fault persists will immediately trigger a return
// to the Idle state within milliseconds.
//
// Expects soft_timer, can, interrupts to be enabled.
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
