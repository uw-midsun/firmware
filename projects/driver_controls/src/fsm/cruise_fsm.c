#include "cruise_fsm.h"
#include "input_event.h"
// Desired behavior:

// On
// Cancel
// Off
// Resume
// Speed+
// Speed-

// Clears target speed
FSM_DECLARE_STATE(cruise_off);
// Retains target speed
FSM_DECLARE_STATE(cruise_idle);
FSM_DECLARE_STATE(cruise_on);

FSM_STATE_TRANSITION(cruise_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_ON, cruise_idle);
}

// Actually marked as "on"
FSM_STATE_TRANSITION(cruise_idle) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, cruise_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
}

// Cruise resumed
FSM_STATE_TRANSITION(cruise_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, cruise_idle);
}
