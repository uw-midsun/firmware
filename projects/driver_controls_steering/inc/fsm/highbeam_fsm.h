#pragma once
// Highbeams FSM for the steering stalk
//
// This FSM will add the new state to Steering Output
// Console Headlight FSM will receive this and turn on Hazards
// This will keep both FSMs in the same state
#include "event_arbiter.h"
#include "fsm.h"

StatusCode highbeam_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
