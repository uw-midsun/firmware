#include "lights_signal_fsm.h"

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

static bool prv_guard_signal_left(const FSM *fsm, const Event *e, void *context) {
  return (e->data == LIGHTS_EVENT_SIGNAL_MODE_LEFT);
}

static bool prv_guard_signal_right(const FSM *fsm, const Event *e, void *context) {
  return (e->data == LIGHTS_EVENT_SIGNAL_MODE_RIGHT);
}

static bool prv_guard_signal_hazard(const FSM *fsm, const Event *e, void *context) {
  return (e->data == LIGHTS_EVENT_SIGNAL_MODE_HAZARD);
}

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_left, state_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_right, state_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_hazard, state_hazard_signal);
}

FSM_STATE_TRANSITION(state_left_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_right, state_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_hazard,
                             state_hazard_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_left, state_none);
}

FSM_STATE_TRANSITION(state_hazard_left_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_right,
                             state_hazard_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_left, state_hazard_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_hazard, state_left_signal);
}

FSM_STATE_TRANSITION(state_right_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_right, state_none);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_hazard,
                             state_hazard_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_left, state_left_signal);
}

FSM_STATE_TRANSITION(state_hazard_right_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_hazard, state_right_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_right, state_hazard_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_left,
                             state_hazard_left_signal);
}

FSM_STATE_TRANSITION(state_hazard_signal) {
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_OFF, prv_guard_signal_hazard, state_none);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_left,
                             state_hazard_left_signal);
  FSM_ADD_GUARDED_TRANSITION(LIGHTS_EVENT_SIGNAL_ON, prv_guard_signal_right,
                             state_hazard_right_signal);
}

// Deactivates the current blinker.
static void prv_state_none_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFsm *lights_signal_fsm = (LightsSignalFsm *)context;
  lights_blinker_deactivate(&lights_signal_fsm->blinker);
}

// Makes left signal blink.
static void prv_state_left_signal_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFsm *lights_signal_fsm = (LightsSignalFsm *)context;
  lights_blinker_activate(&lights_signal_fsm->blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
}

// Makes right signal blink.
static void prv_state_right_signal_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFsm *lights_signal_fsm = (LightsSignalFsm *)context;
  lights_blinker_activate(&lights_signal_fsm->blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);
}

// Makes both left and right signals blink (HAZARD state).
static void prv_state_hazard_signal_output(FSM *fsm, const Event *e, void *context) {
  LightsSignalFsm *lights_signal_fsm = (LightsSignalFsm *)context;
  lights_blinker_activate(&lights_signal_fsm->blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);
}

StatusCode lights_signal_fsm_init(LightsSignalFsm *lights_signal_fsm,
                                  const LightsBlinkerDuration blinker_duration, uint32_t count) {
  fsm_state_init(state_none, prv_state_none_output);
  fsm_state_init(state_left_signal, prv_state_left_signal_output);
  fsm_state_init(state_right_signal, prv_state_right_signal_output);
  fsm_state_init(state_hazard_signal, prv_state_hazard_signal_output);
  fsm_state_init(state_hazard_left_signal, prv_state_hazard_signal_output);
  fsm_state_init(state_hazard_right_signal, prv_state_hazard_signal_output);

  status_ok_or_return(lights_blinker_init(&lights_signal_fsm->blinker, blinker_duration, count));
  fsm_init(&lights_signal_fsm->fsm, "Lights Signal FSM", &state_none, lights_signal_fsm);
  return STATUS_CODE_OK;
}

StatusCode lights_signal_fsm_process_event(LightsSignalFsm *lights_signal_fsm, const Event *event) {
  fsm_process_event(&lights_signal_fsm->fsm, event);
  if (event->id == LIGHTS_EVENT_SYNC_RX) {
    return lights_blinker_force_on(&lights_signal_fsm->blinker);
  }
  return STATUS_CODE_OK;
}
