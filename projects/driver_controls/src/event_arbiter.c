#include <stdio.h>

#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

static FSM *s_driver_fsms[EVENT_ARBITER_MAX_FSMS];
static EventArbiterCheck s_event_checks[EVENT_ARBITER_MAX_FSMS];

static uint8_t s_num_active_fsms = 0;

static bool prv_event_permitted(Event *e) {
  for (uint8_t i = 0; i < s_num_active_fsms; i++) {
    if (s_event_checks[i] != NULL && !s_event_checks[i](e)) {
      return false;
    }
  }
  return true;
}

StatusCode event_arbiter_init() {
  for (uint8_t i = 0; i < EVENT_ARBITER_MAX_FSMS; i++) {
    s_driver_fsms[i] = NULL;
    s_event_checks[i] = NULL;
  }
  s_num_active_fsms = 0;
}

EventArbiterCheck *event_arbiter_add_fsm(FSM *fsm, EventArbiterCheck default_checker) {
  if (s_num_active_fsms == EVENT_ARBITER_MAX_FSMS) {
    return NULL;
  }

  s_driver_fsms[s_num_active_fsms] = fsm;
  s_event_checks[s_num_active_fsms] = default_checker;

  return &s_event_checks[s_num_active_fsms++];
}

bool event_arbiter_process_event(Event *e) {
  bool transitioned = false;

  if (prv_event_permitted(e)) {
    for (uint8_t i = 0; i < s_num_active_fsms; i++) {
      transitioned |= fsm_process_event(s_driver_fsms[i], e);
    }
  }
  return transitioned;
}
