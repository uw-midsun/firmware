#pragma once

#include <stdint.h>

#include "can_ack.h"
#include "fsm.h"
#include "relay_id.h"
#include "status.h"

#define RELAY_FSM_MAX_RETRIES 2
#define NUM_RELAY_FSMS NUM_RELAY_IDS

// Initializes the relay_fsm module.
void relay_fsm_init(void);

// Creates a new relay FSM instance for a RelayId.
StatusCode relay_fsm_create(FSM* fsm, RelayId relay_id, const char* fsm_name,
                            uint32_t ack_device_bitset);

// Sets |e| with the event to open the relay corresponding to |relay_id|.
StatusCode relay_fsm_open_event(RelayId relay_id, Event* e);

// Sets |e| with the event to close the relay corresponding to |relay_id|.
StatusCode relay_fsm_close_event(RelayId relay_id, Event* e);
