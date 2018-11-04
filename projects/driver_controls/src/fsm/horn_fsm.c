#include "horn_fsm.h"
#include "can_transmit.h"
#include "event_arbiter.h"
#include "exported_enums.h"
#include "input_event.h"

// Horn FSM state definitions

FSM_DECLARE_STATE(state_horn_off);
FSM_DECLARE_STATE(state_horn_on);

// Horn FSM transition table definitions

FSM_STATE_TRANSITION(state_horn_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HORN_PRESSED, state_horn_on);
}

FSM_STATE_TRANSITION(state_horn_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HORN_RELEASED, state_horn_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, state_horn_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, state_horn_off);
}

// Horn FSM output function
static void prv_horn_off_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_HORN(EE_HORN_STATE_OFF);
}

static void prv_horn_on_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_HORN(EE_HORN_STATE_ON);
}

StatusCode horn_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_horn_off, prv_horn_off_output);
  fsm_state_init(state_horn_on, prv_horn_on_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Horn FSM", &state_horn_off, guard);

  return STATUS_CODE_OK;
}
