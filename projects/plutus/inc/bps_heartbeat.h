#pragma once
// BPS heartbeat - sends periodic heartbeat CAN messages containing the fault state
// Requires soft timers, CAN to be initialized
#include "sequenced_relay.h"

typedef enum {
  BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH = 0,
  BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE,
  BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC,
  BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT,
  NUM_BPS_HEARTBEAT_FAULT_SOURCES,
} BpsHeartbeatFaultSource;

typedef struct BpsHeartbeatStorage {
  SequencedRelayStorage *relay;
  uint32_t period_ms;
  uint32_t expected_bitset;
  uint8_t fault_bitset;
} BpsHeartbeatStorage;

// Sets up the periodic heartbeat message
StatusCode bps_heartbeat_init(BpsHeartbeatStorage *storage, SequencedRelayStorage *relay,
                              uint32_t period_ms, uint32_t expected_bitset);

// Marks the specific fault source as faulted.
// Immediately sends a heartbeat message with the fault state and opens the HV relays.
StatusCode bps_heartbeat_raise_fault(BpsHeartbeatStorage *storage, BpsHeartbeatFaultSource source);

// Explicitly clear previous faults from this source. The HV relays will not be changed.
// The BPS heartbeat state will not be cleared until all fault sources are cleared.
StatusCode bps_heartbeat_clear_fault(BpsHeartbeatStorage *storage, BpsHeartbeatFaultSource source);
