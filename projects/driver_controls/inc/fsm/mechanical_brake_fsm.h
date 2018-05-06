#pragma once

// Keeps track of whether the mechanical brake is being pressed or not

// Coast, drive, and any events involving cruise control will not be acknowledged while
// the mechanical brake is pressed

// Direction selector events will not be acknowledged while the mechanical brake is not pressed

#include "event_arbiter.h"
#include "fsm.h"

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage);
