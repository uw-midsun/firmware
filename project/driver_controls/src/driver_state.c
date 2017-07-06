#include <stdio.h>

#include "driver_state.h"

#define MAX_FSMS 10

typedef bool (*TransitionCheck)(Event *e);

static FSM *s_driver_fsms[MAX_FSMS];
static uint8_t s_active_fsms = 0;

static bool prv_get_permit(Event *e) {
  bool transitioned = 1;

  for (uint8_t i = 0; i < s_active_fsms; i++) {
    transitioned &= ((TransitionCheck)s_driver_fsms[i]->context)(e);
  }

  return transitioned;
}

StatusCode driver_state_add_fsm(FSM *fsm, DriverFSMInit driver_fsm_init) {
  if (s_active_fsms == 10) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  driver_fsm_init(fsm, NULL);
  s_driver_fsms[s_active_fsms] = fsm;
  s_active_fsms++;
  return STATUS_CODE_OK;
}

bool driver_state_process_event(Event *e) {
  if (prv_get_permit(e)) {
    for (uint8_t i = 0; i < s_active_fsms; i++) {
      if (fsm_process_event(s_driver_fsms[i], e)) {
        return true;
      }
    }
  }

  return false;
}
