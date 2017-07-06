#include <stdio.h>

#include "driver_state.h"

#define MAX_FSMS 10

typedef bool (*TransitionCheck)(Event *e);

static FSM s_driver_fsms[MAX_FSMS];
static uint8_t s_active_fsms = 0;

static bool prv_get_permit(Event *e) {
  bool transitioned = 1;

  for (uint8_t i = 0; i < s_active_fsms; i++) {
    transitioned &= ((TransitionCheck)s_driver_fsms[i].context)(e);
  }

  return transitioned;
}

void driver_state_init(FSMGroup *fsm_group) {
  fsm_group->power = &s_driver_fsms[0];
  fsm_group->pedal = &s_driver_fsms[1];
  fsm_group->direction = &s_driver_fsms[2];
  fsm_group->turn_signal = &s_driver_fsms[3];
  fsm_group->hazard_light = &s_driver_fsms[4];
  fsm_group->mechanical_brake = &s_driver_fsms[5];
}


StatusCode driver_state_add_fsm(DriverFSMInit driver_fsm_init) {
  driver_fsm_init(&s_driver_fsms[s_active_fsms], NULL);
  s_active_fsms++;
  return STATUS_CODE_OK;
}

bool driver_state_process_event(Event *e) {
  if (prv_get_permit(e)) {
    for (uint8_t i = 0; i < s_active_fsms; i++) {
      if (fsm_process_event(&s_driver_fsms[i], e)) {
        return true;
      }
    }
  }

  return false;
}
