#pragma once
// BPS heartbeat - sends periodic heartbeat CAN messages containing the fault state
// Requires soft timers, CAN to be initialized
#include "sequenced_relay.h"

typedef struct BpsHeartbeatStorage {
  SequencedRelayStorage *relay;
  uint32_t period_ms;
  uint32_t expected_bitset;
  bool faulted;
} BpsHeartbeatStorage;

// Sets up the periodic heartbeat message
StatusCode bps_heartbeat_init(BpsHeartbeatStorage *storage, SequencedRelayStorage *relay, uint32_t period_ms, uint32_t expected_bitset);

// Immediately sends a heartbeat message with the fault state and opens the HV relays.
StatusCode bps_heartbeat_raise_fault(BpsHeartbeatStorage *storage);

// Explicitly clear any previous faults. The HV relays will not be changed.
StatusCode bps_heartbeat_clear_fault(BpsHeartbeatStorage *storage);
