#pragma once

// Interface for managing raised events

// Each time an event is popped from the event queue, the event will be checked against
// the current states of all active FSMs to determine whether it will be processed or
// discarded. This is to prevent situations like processing a gear shift while the brake is
// not pressed, which would be dangerous for the driver.

#include "fsm.h"
#include "status.h"

// Arbitrary FSM cap
#define EVENT_ARBITER_MAX_FSMS 10

// Typedef for the FSM arbitration functions
typedef bool (*EventArbiterCheck)(const Event *e);

typedef void (*EventArbiterCANOutput)(uint8_t device_id, uint8_t device_state, uint16_t data);

// Initializes the event arbiter to the default state with a given output function
StatusCode event_arbiter_init(EventArbiterCANOutput output);

// Registers an FSM and sets the context pointer to point to the given event check function
// NULL pointers are treated as no-ops that return true
EventArbiterCheck *event_arbiter_add_fsm(FSM *fsm, EventArbiterCheck default_checker);

// Process an event if allowed in the current state
bool event_arbiter_process_event(Event *e);

StatusCode event_arbiter_can_output(uint8_t device_id, uint8_t device_state, uint16_t data);
