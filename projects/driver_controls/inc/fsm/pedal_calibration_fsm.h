#pragma once
//
#include "event_arbiter.h"
#include "fsm.h"

StatusCode pedal_calibration_fsm_init(FSM *fsm, EventArbiterStorage *storage);
