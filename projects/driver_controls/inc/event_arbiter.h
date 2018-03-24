#pragma once

// Interface for managing raised events

// Each time an event is popped from the event queue, the event will be checked against
// the current states of all active FSMs to determine whether it will be processed or
// discarded. This is to prevent situations like processing a gear shift while the brake is
// not pressed, which would be dangerous for the driver.

#include "fsm.h"
#include "objpool.h"
#include "status.h"

// Arbitrary FSM cap
#define EVENT_ARBITER_MAX_FSMS 10

// Returns whether the given event should be processed by any FSMs
typedef bool (*EventArbiterGuardFn)(const Event *e);

typedef struct EventArbiterGuard {
  FSM *fsm;
  EventArbiterGuardFn guard_fn;
} EventArbiterGuard;

typedef struct EventArbiterStorage {
  size_t num_registered_arbiters;
  EventArbiterGuard arbiters[EVENT_ARBITER_MAX_FSMS];
} EventArbiterStorage;

// Initializes the event arbiter to the default state with a given output function
StatusCode event_arbiter_init(EventArbiterStorage *storage);

// Registers an FSM and initializes an arbiter with the given event check function
// A NULL event check allows all events by default.
EventArbiterGuard *event_arbiter_add_fsm(EventArbiterStorage *storage, FSM *fsm,
                                         EventArbiterGuardFn guard_fn);

StatusCode event_arbiter_set_guard_fn(EventArbiterGuard *guard, EventArbiterGuardFn guard_fn);

// Process an event if allowed by the registered arbiters
bool event_arbiter_process_event(const EventArbiterStorage *storage, const Event *e);
