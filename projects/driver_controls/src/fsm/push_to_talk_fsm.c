#include "push_to_talk_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Push-to-Talk FSM state definitions

FSM_DECLARE_STATE(state_active);
FSM_DECLARE_STATE(state_inactive);

// Push-to-Talk transition table defintions

FSM_STATE_TRANSITION(state_active) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PUSH_TO_TALK, state_inactive);
}

FSM_STATE_TRANSITION(state_inactive) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PUSH_TO_TALK, state_active);
}

// Push-to-Talk output function

static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  PushToTalkFSMState push_to_talk_state = PUSH_TO_TALK_FSM_STATE_INACTIVE;

  if (fsm->current_state == &state_active) {
    push_to_talk_state = PUSH_TO_TALK_FSM_STATE_ACTIVE;
  }

  (void)push_to_talk_state;
  // Previous: Output PTT
}

StatusCode push_to_talk_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_active, prv_state_output);
  fsm_state_init(state_inactive, prv_state_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "push_to_talk_fsm", &state_inactive, guard);

  return STATUS_CODE_OK;
}
