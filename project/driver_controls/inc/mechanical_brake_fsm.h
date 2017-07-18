#pragma once

// Keeps track of whether the mechanical brake is being pressed or not

// Coast, drive, and any events involving cruise control will not be acknowledged while
// the machanical brake is pressed

// Direction selector events will not be acknowledged while the mechanical brake is not pressed

#include "fsm.h"

void mechanical_brake_fsm_init(FSM *fsm);
