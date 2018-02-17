#pragma once

// Monitors the Push-to-Talk system

#include "event_arbiter.h"
#include "fsm.h"

typedef enum {
  PUSH_TO_TALK_FSM_STATE_ACTIVE,
  PUSH_TO_TALK_FSM_STATE_INACTIVE,
} PushToTalkFSMState;

StatusCode push_to_talk_fsm_init(FSM *fsm, EventArbiterStorage *storage);
