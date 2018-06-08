#pragma once
// Handles hazards - this FSM will output hazard state over CAN.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode hazard_light_fsm_init(FSM *fsm, EventArbiterStorage *storage);
