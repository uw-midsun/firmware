#include "horn_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Horn FSM state definitions

FSM_DECLARE_STATE(state_horn_off);
FSM_DECLARE_STATE(state_horn_on);

// Horn FSM transition table definitions

FSM_STATE_TRANSITION(state_horn_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HORN, state_horn_on);
}

FSM_STATE_TRANSITION(state_horn_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HORN, state_horn_off);
}

// Horn FSM output function
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  HornFSMState horn_state = HORN_FSM_STATE_OFF;

  if (fsm->current_state == &state_horn_on) {
    horn_state = HORN_FSM_STATE_ON;
  }

  (void)horn_state;

  // EventArbiterOutputData data = {
  //   .id = CAN_OUTPUT_MESSAGE_HORN, //
  //   .state = horn_state, //
  //   .data = 0 //
  // };

  // event_arbiter_output(data);
}

StatusCode horn_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_horn_off, prv_state_output);
  fsm_state_init(state_horn_on, prv_state_output);

  EventArbiter *arbiter = event_arbiter_add_fsm(storage, fsm, NULL);

  if (arbiter == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "horn_fsm", &state_horn_off, arbiter);

  return STATUS_CODE_OK;
}
