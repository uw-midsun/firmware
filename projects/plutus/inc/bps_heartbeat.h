#pragma once
// BPS heartbeat - sends periodic heartbeat CAN messages containing the fault state
// Requires soft timers, CAN, event queue to be initialized
//
// Raises PLUTUS_EVENT_BPS_FAULT on fault with the data field set to the fault bitset
#include "exported_enums.h"
#include "sequenced_relay.h"

// Arbitrary multiplier to the heartbeat period for initial startup delay
// This is necessary because Chaos takes some time to initialize before it can ACK
#define BPS_HEARTBEAT_STARTUP_DELAY_MULTIPLIER 5

typedef struct BpsHeartbeatStorage {
  SequencedRelayStorage *relay;
  uint32_t period_ms;
  uint32_t expected_bitset;
  uint8_t fault_bitset;
  uint8_t ack_fail_counter;
} BpsHeartbeatStorage;

// Sets up the periodic heartbeat message
StatusCode bps_heartbeat_init(BpsHeartbeatStorage *storage, SequencedRelayStorage *relay,
                              uint32_t period_ms, uint32_t expected_bitset);

// Marks the specific fault source as faulted.
// Immediately sends a heartbeat message with the fault state and opens the HV relays.
StatusCode bps_heartbeat_raise_fault(BpsHeartbeatStorage *storage,
                                     EEBpsHeartbeatFaultSource source);

// Explicitly clear previous faults from this source. The HV relays will not be changed.
// The BPS heartbeat state will not be cleared until all fault sources are cleared.
StatusCode bps_heartbeat_clear_fault(BpsHeartbeatStorage *storage,
                                     EEBpsHeartbeatFaultSource source);
