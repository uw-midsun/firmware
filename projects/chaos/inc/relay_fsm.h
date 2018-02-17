#pragma once

#include "can_ack.h"
#include "fsm.h"
#include "relay_id.h"
#include "status.h"

#define RELAY_FSM_MAX_RETRIES 3
#define NUM_RELAY_FSMS NUM_RELAY_IDS

void relay_fsm_init(void);

StatusCode relay_fsm_create(FSM *fsm, RelayId relay_id, const char *fsm_name,
                            uint32_t ack_device_bitset);
