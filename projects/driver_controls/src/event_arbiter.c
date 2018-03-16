#include "event_arbiter.h"
#include <stddef.h>

StatusCode event_arbiter_init(EventArbiterStorage *storage) {
  storage->num_registered_arbiters = 0;

  return STATUS_CODE_OK;
}

EventArbiter *event_arbiter_add_fsm(EventArbiterStorage *storage, FSM *fsm,
                                    EventArbiterCheck event_check_fn) {
  if (storage->num_registered_arbiters >= EVENT_ARBITER_MAX_FSMS) {
    return NULL;
  }

  EventArbiter *arbiter = &storage->arbiters[storage->num_registered_arbiters++];

  arbiter->fsm = fsm;
  arbiter->event_check_fn = event_check_fn;

  return arbiter;
}

StatusCode event_arbiter_set_event_check(EventArbiter *arbiter, EventArbiterCheck event_check_fn) {
  arbiter->event_check_fn = event_check_fn;

  return STATUS_CODE_OK;
}

bool event_arbiter_process_event(const EventArbiterStorage *storage, const Event *e) {
  // Check if any of the arbiters block this event
  for (size_t i = 0; i < storage->num_registered_arbiters; i++) {
    if (storage->arbiters[i].event_check_fn != NULL && !storage->arbiters[i].event_check_fn(e)) {
      return false;
    }
  }

  bool transitioned = false;
  // We didn't get blocked by any of the arbiters - process the event
  for (size_t i = 0; i < storage->num_registered_arbiters; i++) {
    transitioned |= fsm_process_event(storage->arbiters[i].fsm, e);
  }

  return transitioned;
}
