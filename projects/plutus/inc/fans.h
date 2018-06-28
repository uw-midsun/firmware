#pragma once
// Fan control based on events
// Requires event queue, GPIO, sequenced relays to be initialized.
//
// Fans will be on if relays are closed or latched after fault unless explicitly cleared.
#include <stddef.h>
#include "event_queue.h"
#include "gpio.h"
#include "plutus_cfg.h"
#include "sequenced_relay.h"

typedef struct FansSettings {
  SequencedRelayStorage *relays;
  GPIOAddress fans[PLUTUS_CFG_NUM_FANS];
} FansSettings;

StatusCode fans_init(const FansSettings *settings);

// Control whether fans should be on based on input events
bool fans_process_event(const Event *e);
