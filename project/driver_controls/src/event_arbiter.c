#include <stdio.h>

#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Arbitrary FSM cap
#define MAX_FSMS 10

static FSM *s_driver_fsms[MAX_FSMS];
static EventArbiterCheck s_event_checks[MAX_FSMS];  // Function pointer array to FSM arbiter functions

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

StatusCode event_arbiter_add_fsm(FSM *fsm, DriverFSMInit driver_fsm_init) {
  if (s_num_active_fsms == MAX_FSMS) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  // Initialize the FSM with a pointer to the corresponding entry in the function pointer array
  driver_fsm_init(fsm, &s_event_checks[s_num_active_fsms]);
  s_driver_fsms[s_num_active_fsms] = fsm;
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
