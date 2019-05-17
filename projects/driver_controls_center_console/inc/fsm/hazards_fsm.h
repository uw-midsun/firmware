#pragma once
// Handles hazards - this FSM will output hazard state over CAN.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode hazards_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
