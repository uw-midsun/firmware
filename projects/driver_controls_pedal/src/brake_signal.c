// We handle the following events:
//
// - PEDAL_EVENT_INPUT_POWER_STATE_*
// - PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_*
// - PEDAL_EVENT_INPUT_DIRECTION_STATE_*
// - PEDAL_EVENT_INPUT_PEDAL_*
// - PEDAL_EVENT_INPUT_SPEED_UPDATE
#include "brake_signal.h"

#include <stdbool.h>
#include <string.h>

#include "can_transmit.h"
#include "exported_enums.h"
#include "input_event.h"

typedef struct BrakeSignalInputState {
  // prev_signal_state keeps track of whether the brakes were previously turned
  // on. This is intended to be used such that we only send CAN messages when
  // the Brake signal state changes.
  bool prev_signal_state;
  union {
    struct {
      // power_state is set if we are in the Drive state
      uint8_t power_state : 1;
      // mech_brake is set if the Mechanical brake is set
      uint8_t mech_brake : 1;
      // direction is set if we are not in Neutral
      uint8_t direction : 1;
      // throttle is set if the throttle is in the Brake region
      uint8_t throttle : 1;
      // speed is set if our speed exceeds the minimum speed at which we turn
      // on the brakes (BRAKE_SIGNAL_MIN_SPEED_CMS)
      uint8_t speed : 1;
    };
    uint8_t raw;
  } input_state;
} BrakeSignalInputState;

// Used to cache the current state of the various variables we consider to
// determine whether to turn the brakes on or off.
static BrakeSignalInputState s_factor_state = { 0 };

StatusCode brake_signal_init(void) {
  memset(&s_factor_state, 0, sizeof(s_factor_state));

  return STATUS_CODE_OK;
}

bool brake_signal_process_event(const Event *e) {
  bool processed = true;
  switch (e->id) {
    // Events raised from the Power State FSM
    case PEDAL_EVENT_INPUT_POWER_STATE_OFF:
      // Fall through
    case PEDAL_EVENT_INPUT_POWER_STATE_DRIVE:
      // Fall through
    case PEDAL_EVENT_INPUT_POWER_STATE_CHARGE:
      // Fall through
    case PEDAL_EVENT_INPUT_POWER_STATE_FAULT:
      s_factor_state.input_state.power_state = e->id == PEDAL_EVENT_INPUT_POWER_STATE_DRIVE;
      break;

    case PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED:
      // Fall through
    case PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_RELEASED:
      s_factor_state.input_state.mech_brake = e->id == PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED;
      break;

    // Events raised from the Direction FSM
    case PEDAL_EVENT_INPUT_DIRECTION_STATE_NEUTRAL:
      // Fall through
    case PEDAL_EVENT_INPUT_DIRECTION_STATE_FORWARD:
      // Fall through
    case PEDAL_EVENT_INPUT_DIRECTION_STATE_REVERSE:
      s_factor_state.input_state.direction = e->id != PEDAL_EVENT_INPUT_DIRECTION_STATE_NEUTRAL;
      break;

    // Events raised from the Pedal FSM
    case PEDAL_EVENT_INPUT_PEDAL_ACCEL:
      // Fall through
    case PEDAL_EVENT_INPUT_PEDAL_COAST:
      // Fall through
    case PEDAL_EVENT_INPUT_PEDAL_BRAKE:
      s_factor_state.input_state.throttle = e->id == PEDAL_EVENT_INPUT_PEDAL_BRAKE;
      break;

    //
    case PEDAL_EVENT_INPUT_SPEED_UPDATE:
      s_factor_state.input_state.speed = e->data >= BRAKE_SIGNAL_MIN_SPEED_CMS;
      break;

    default:
      processed = false;
  }

  // (Power == Drive) &&
  //  (Mech Brake == Engaged ||
  //   (Direction != Neutral && Throttle == Brake && Speed > x))
  bool signal_on = s_factor_state.input_state.power_state &&
                   (s_factor_state.input_state.mech_brake ||
                    (s_factor_state.input_state.direction && s_factor_state.input_state.throttle &&
                     s_factor_state.input_state.speed));
  // We only send a new CAN message when the state changes
  if (signal_on != s_factor_state.prev_signal_state) {
    s_factor_state.prev_signal_state = signal_on;

    CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_BRAKES, signal_on);
  }

  return processed;
}
