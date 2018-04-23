#pragma once
//
#include "event_arbiter.h"
#include "fsm.h"
#include "pedal_calibration.h"

StatusCode pedal_calibration_fsm_init(FSM *fsm, PedalCalibrationStorage *calibration_storage);
