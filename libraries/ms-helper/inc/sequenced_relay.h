#pragma once
// Handles relay CAN requests and sequencing
// Requires CAN, soft timers to be initialized.
//
// Sequences relay closing with a slight delay to offset the high make current of the HV relay coil.
// We also assume the relays are active-high.
#include "gpio.h"
#include "relay_rx.h"
#include "soft_timer.h"
#include "status.h"

typedef struct SequencedRelaySettings {
  SystemCanMessage can_message;
  GPIOAddress left_relay;
  GPIOAddress right_relay;
  // Delay between left and right relays closing
  uint32_t delay_ms;
} SequencedRelaySettings;

typedef struct SequencedRelayStorage {
  SequencedRelaySettings settings;
  SoftTimerID delay_timer;
  RelayRxStorage relay_rx;
} SequencedRelayStorage;

// |storage| should persist
StatusCode sequenced_relay_init(SequencedRelayStorage *storage, const SequencedRelaySettings *settings);
