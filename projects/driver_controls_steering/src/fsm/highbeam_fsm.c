#include "hazards_fsm.h"
#include "sc_input_event.h"
#include "steering_output.h"

// Only keep track of highbeam on/off state
FSM_DECLARE_STATE(highbeam_on);
FSM_DECLARE_STATE(highbeam_off);

FSM_STATE_TRANSITION(highbeams_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED, highbeam_on);
}

FSM_STATE_TRANSITION(highbeams_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HIGH_BEAM_FWD_RELEASED, highbeam_off);
}

static void prv_highbeam_off_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_OFF);
}

static void prv_highbeam_on_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_ON);
}

StatusCode highbeam_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_init_state(highbeam_off, prv_highbeam_off_output);
  fsm_init_state(highbeam_on, prv_highbeam_on_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Highbeam FSM", &highbeam_off, guard);

  return STATUS_CODE_OK;
}
