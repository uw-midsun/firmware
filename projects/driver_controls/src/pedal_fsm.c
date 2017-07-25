// Some events can be raised despite the FSM being in the corresponding state (such as
// INPUT_EVENT_PEDAL_BRAKE being called while the pedal FSM is in the brake state). Even though
// these events will not cause state transitions, they still may have data that needs to be passed
// to CAN. As a result, these repeated events are dealt with by transitioning back into their own
// state

#include "pedal_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "log.h"

// Pedal FSM state definitions

FSM_DECLARE_STATE(state_brake);
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);
FSM_DECLARE_STATE(state_cruise_control);

// Pedal FSM transition table definitions

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  // Press the gas pedal to start moving the car
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  // Since the cruise control increase/decrease events have information that needs to be output to
  // CAN, they will cause repeat transitions
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_INC, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_DEC, state_cruise_control);

  // Cruise control exits either through hitting the cruise control switch or by engaging the
  // mechanical brake
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_brake);
}

// Pedal FSM output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  InputEventData data;
  data.raw = e->data;

  State *current_state = fsm->current_state;

  if (current_state == &state_brake) {
    data.components.state = PEDAL_FSM_STATE_BRAKE;
  } else if (current_state == &state_coast) {
    data.components.state = PEDAL_FSM_STATE_COAST;
  } else if (current_state == &state_driving) {
    data.components.state = PEDAL_FSM_STATE_DRIVING;
  } else if (current_state == &state_cruise_control) {
    data.components.state = PEDAL_FSM_STATE_CRUISE_CONTROL;
  }

  event_raise(INPUT_EVENT_CAN_ID_PEDAL, data.raw);
}

StatusCode pedal_fsm_init(FSM *fsm) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_state_output);

  void *context = event_arbiter_add_fsm(fsm, NULL);

  if (context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "pedal_fsm", &state_brake, context);

  return STATUS_CODE_OK;
}
