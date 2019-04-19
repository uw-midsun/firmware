#pragma once
// Handles horn events - this FSM will output horn state over CAN.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode horn_fsm_init(FSM *fsm, EventArbiterStorage *storage);
