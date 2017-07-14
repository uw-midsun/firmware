#pragma once

// Interface for managing raised events

// Each time an event is popped from the event queue, the event will be checked against
// the current states of all active FSMs to determine whether it will be processed or
// discarded. This is to prevent situations like processing a gear shift while the brake is
// not pressed, which would be dangerous for the driver.

#include "fsm.h"
#include "status.h"

typedef void (*DriverFSMInit)(FSM *fsm, void *context);

// Initializes an FSM with the given init function
StatusCode event_arbiter_add_fsm(FSM *fsm, DriverFSMInit driver_fsm_init);

// Process an event if allowed in the current state
bool event_arbiter_process_event(Event *e);
