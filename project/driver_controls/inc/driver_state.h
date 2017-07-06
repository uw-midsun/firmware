#pragma once
#include "event_queue.h"
#include "fsm_state.h"
#include "status.h"

typedef void (*DriverFSMInit)(FSM *power_fsm, void *context);

// Initializes an FSM with the given init function
StatusCode driver_state_add_fsm(FSM *fsm, DriverFSMInit driver_fsm_init);

// Process an event if allowed in the current state
bool driver_state_process_event(Event *e);
