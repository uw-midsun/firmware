#include "lights_signal_fsm.h"
#include <stdio.h>

// No signals are active.
FSM_DECLARE_STATE(state_none);
// Left signal active.
FSM_DECLARE_STATE(state_left_signal);
// Right signal active.
FSM_DECLARE_STATE(state_right_signal);
// Hazard button pressed.
FSM_DECLARE_STATE(state_hazard_signal);
// Hazard button pressed, while left signal active.
FSM_DECLARE_STATE(state_hazard_left_signal);
// Hazard button pressed, while right signal active.
FSM_DECLARE_STATE(state_hazard_right_signal);

// Guard for turning lights ON.
static bool prv_guard_signal_cmd_on(const FSM *fsm, const Event *e, void *context) {
  return (e->data == LIGHTS_SIGNAL_FSM_CMD_ON);
}

// Guard for turning lights OFF.
static bool prv_guard_signal_cmd_off(const FSM *fsm, const Event *e, void *context) {
  return (e->data == LIGHTS_SIGNAL_FSM_CMD_OFF);
}

// Transition table for state_none.
FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_LEFT, prv_guard_signal_cmd_on, state_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_RIGHT, prv_guard_signal_cmd_on,
                             state_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_HAZARD, prv_guard_signal_cmd_on,
                             state_hazard_signal);
}

// Transition table for state_left_signal.
FSM_STATE_TRANSITION(state_left_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_HAZARD, prv_guard_signal_cmd_on,
                             state_hazard_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_LEFT, prv_guard_signal_cmd_off, state_none);
}

// Transition table for state_hazard_left_signal.
FSM_STATE_TRANSITION(state_hazard_left_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_HAZARD, prv_guard_signal_cmd_off,
                             state_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_LEFT, prv_guard_signal_cmd_off,
                             state_hazard_signal);
}

// Transition table for state_right_signal.
FSM_STATE_TRANSITION(state_right_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_RIGHT, prv_guard_signal_cmd_off, state_none);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_HAZARD, prv_guard_signal_cmd_on,
                             state_hazard_right_signal);
}

// Transition table for state_hazard_right_signal.
FSM_STATE_TRANSITION(state_hazard_right_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_HAZARD, prv_guard_signal_cmd_off,
                             state_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_RIGHT, prv_guard_signal_cmd_off,
                             state_hazard_signal);
}

// Transition table forstate_hazard_signal.
FSM_STATE_TRANSITION(state_hazard_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_HAZARD, prv_guard_signal_cmd_off, state_none);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_LEFT, prv_guard_signal_cmd_on,
                             state_hazard_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_RIGHT, prv_guard_signal_cmd_on,
                             state_hazard_right_signal);
}

// Deactivates the current blinker.
static void prv_state_none_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFSM *lights_signal_fsm = (LightsSignalFSM *)context;
  lights_blinker_deactivate(&lights_signal_fsm->blinker);
}

// Makes left signal blink.
static void prv_state_left_signal_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFSM *lights_signal_fsm = (LightsSignalFSM *)context;
  lights_blinker_activate(&lights_signal_fsm->blinker, LIGHTS_EVENT_SIGNAL_LEFT);
}

// Makes right signal blink.
static void prv_state_right_signal_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFSM *lights_signal_fsm = (LightsSignalFSM *)context;
  lights_blinker_activate(&lights_signal_fsm->blinker, LIGHTS_EVENT_SIGNAL_RIGHT);
}

// Makes both left and right signals blink (HAZARD state).
static void prv_state_hazard_signal_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFSM *lights_signal_fsm = (LightsSignalFSM *)context;
  lights_blinker_activate(&lights_signal_fsm->blinker, LIGHTS_EVENT_SIGNAL_HAZARD);
}

StatusCode lights_signal_fsm_init(LightsSignalFSM *lights_signal_fsm,
                                  LightsBlinkerDuration blinker_duration) {
  fsm_state_init(state_none, prv_state_none_output);
  fsm_state_init(state_left_signal, prv_state_left_signal_output);
  fsm_state_init(state_right_signal, prv_state_right_signal_output);
  fsm_state_init(state_hazard_signal, prv_state_hazard_signal_output);
  fsm_state_init(state_hazard_left_signal, prv_state_hazard_signal_output);
  fsm_state_init(state_hazard_right_signal, prv_state_hazard_signal_output);
  status_ok_or_return(lights_blinker_init(&lights_signal_fsm->blinker, blinker_duration));
  fsm_init(&lights_signal_fsm->fsm, "Lights Signal FSM", &state_none, lights_signal_fsm);
  return STATUS_CODE_OK;
}

StatusCode lights_signal_fsm_process_event(LightsSignalFSM *lights_signal_fsm, const Event *event) {
  fsm_process_event(&lights_signal_fsm->fsm, event);
  return STATUS_CODE_OK;
}
