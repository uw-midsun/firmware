#include <stdio.h>

#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

static FSM *s_driver_fsms[EVENT_ARBITER_MAX_FSMS];
static EventArbiterCheck s_event_checks[EVENT_ARBITER_MAX_FSMS];  // Function pointer array to FSM arbiter functions

static uint8_t s_num_active_fsms = 0;

static bool prv_event_permitted(Event *e) {
  bool permitted;

  for (uint8_t i = 0; i < s_num_active_fsms; i++) {
    permitted = s_event_checks[i](e);

    if (!permitted) {
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

StatusCode event_arbiter_add_fsm(FSM *fsm, EventArbiterCheck default_checker) {
  if (s_num_active_fsms == EVENT_ARBITER_MAX_FSMS) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  s_driver_fsms[s_num_active_fsms] = fsm;
  s_event_checks[s_num_active_fsms] = default_checker;

  fsm->context = &s_event_checks[s_num_active_fsms];
    
  s_num_active_fsms++;
  return STATUS_CODE_OK;
}

bool event_arbiter_process_event(Event *e) {
  if (prv_event_permitted(e)) {
    for (uint8_t i = 0; i < s_num_active_fsms; i++) {
      bool transitioned = fsm_process_event(s_driver_fsms[i], e);
      if (transitioned) {
        return true;
      }
    }
  }
  return false;
}
