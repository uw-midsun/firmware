#pragma once
// Handles relay CAN requests and sequencing
// Requires CAN, soft timers to be initialized.
//
// Sequences relay closing with a slight delay to offset the high make current of the HV relay coil.
// We also assume the relays are active-high.
#include <stdint.h>
#include "can_msg.h"
#include "exported_enums.h"
#include "gpio.h"
#include "relay_rx.h"
#include "soft_timer.h"
#include "status.h"

// Called after both relays have completed sequencing
typedef void (*SequencedRelayUpdateCb)(EERelayState state, void *context);

typedef struct SequencedRelaySettings {
  CANMessageID can_msg_id;
  GPIOAddress left_relay;
  GPIOAddress right_relay;
  // Delay between left and right relays closing
  uint32_t delay_ms;

  SequencedRelayUpdateCb update_cb;
  void *context;
} SequencedRelaySettings;

typedef struct SequencedRelayStorage {
  SequencedRelaySettings settings;
  SoftTimerID delay_timer;
  RelayRxStorage relay_rx;
} SequencedRelayStorage;

// |storage| should persist
StatusCode sequenced_relay_init(SequencedRelayStorage *storage,
                                const SequencedRelaySettings *settings);

// Updates the callback to be run after both relays have completed sequenced
StatusCode sequenced_relay_set_update_cb(SequencedRelayStorage *storage,
                                         SequencedRelayUpdateCb update_cb, void *context);

StatusCode sequenced_relay_set_state(SequencedRelayStorage *storage, EERelayState state);
