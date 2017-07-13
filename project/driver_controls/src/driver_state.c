#include <stdio.h>

#include "driver_state.h"
#include "input_event.h"
#include "log.h"

#define MAX_FSMS 10

static FSM *s_driver_fsms[MAX_FSMS];
static InputEventCheck s_event_checks[MAX_FSMS];

static uint8_t s_active_fsms = 0;

static bool prv_get_permit(Event *e) {
  bool transitioned;

  for (uint8_t i = 0; i < s_active_fsms; i++) {
    transitioned = s_event_checks[i](e);

    if (!transitioned) {
      return false;
    }
  }
  return true;
}

StatusCode driver_state_add_fsm(FSM *fsm, DriverFSMInit driver_fsm_init) {
  if (s_active_fsms == MAX_FSMS) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  driver_fsm_init(fsm, &s_event_checks[s_active_fsms]);
  s_driver_fsms[s_active_fsms] = fsm;
  s_active_fsms++;
  return STATUS_CODE_OK;
}

bool driver_state_process_event(Event *e) {
  if (prv_get_permit(e)) {
    for (uint8_t i = 0; i < s_active_fsms; i++) {
      bool transitioned = fsm_process_event(s_driver_fsms[i], e);
      if (transitioned) {
        return true;
      }
    }
  }
  return false;
}
