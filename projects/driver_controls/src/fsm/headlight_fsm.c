#include "headlight_fsm.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "input_event.h"

FSM_DECLARE_STATE(headlight_off);
FSM_DECLARE_STATE(headlight_lowbeam);
FSM_DECLARE_STATE(headlight_drl);
FSM_DECLARE_STATE(headlight_lowbeam_highbeam);
FSM_DECLARE_STATE(headlight_drl_highbeam);
FSM_DECLARE_STATE(headlight_highbeam);

FSM_STATE_TRANSITION(headlight_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, headlight_lowbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DRL, headlight_drl);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED, headlight_highbeam);
}

FSM_STATE_TRANSITION(headlight_lowbeam) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DRL, headlight_drl);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
                      headlight_lowbeam_highbeam);
 
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, headlight_off);
}

FSM_STATE_TRANSITION(headlight_drl) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, headlight_lowbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DRL, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
                      headlight_drl_highbeam);
 
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, headlight_off);
}

FSM_STATE_TRANSITION(headlight_lowbeam_highbeam) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, headlight_highbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DRL, headlight_drl_highbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, headlight_lowbeam);
 
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, headlight_off);
}

FSM_STATE_TRANSITION(headlight_drl_highbeam) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, headlight_lowbeam_highbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DRL, headlight_highbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, headlight_drl);
 
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, headlight_off);
}

FSM_STATE_TRANSITION(headlight_highbeam) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, headlight_lowbeam_highbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DRL, headlight_drl_highbeam);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, headlight_off);
 
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, headlight_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, headlight_off);
}

static void prv_headlight_off_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_OFF);
  event_raise(INPUT_EVENT_HEADLIGHT_STATE_OFF, 0);
}

static void prv_headlight_lowbeam_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_ON);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_OFF);
  event_raise(INPUT_EVENT_HEADLIGHT_STATE_LOWBEAM, 0);
}

static void prv_headlight_drl_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_ON);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_OFF);
  event_raise(INPUT_EVENT_HEADLIGHT_STATE_DRL, 0);
}

static void prv_headlight_highbeam_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_ON);
  event_raise(INPUT_EVENT_HEADLIGHT_STATE_HIGHBEAM, 0);
}

StatusCode headlight_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(headlight_off, prv_headlight_off_output);
  fsm_state_init(headlight_lowbeam, prv_headlight_lowbeam_output);
  fsm_state_init(headlight_drl, prv_headlight_drl_output);
  fsm_state_init(headlight_lowbeam_highbeam, prv_headlight_highbeam_output);
  fsm_state_init(headlight_drl_highbeam, prv_headlight_highbeam_output);
  fsm_state_init(headlight_highbeam, prv_headlight_highbeam_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Headlight FSM", &headlight_off, guard);

  return STATUS_CODE_OK;
}
