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
  bool prev_signal_state;
  union {
    struct {
      uint8_t power_state : 1;
      uint8_t mech_brake : 1;
      uint8_t direction : 1;
      uint8_t throttle : 1;
      uint8_t speed : 1;
    };
    uint8_t raw;
  } input_state;
} BrakeSignalInputState;

static BrakeSignalInputState s_brake_state = { 0 };

StatusCode brake_signal_init(void) {
  memset(&s_brake_state, 0, sizeof(s_brake_state));

  return STATUS_CODE_OK;
}

bool brake_signal_process_event(const Event *e) {
  bool processed = true;
  switch (e->id) {
    case PEDAL_EVENT_INPUT_POWER_STATE_OFF:
    case PEDAL_EVENT_INPUT_POWER_STATE_DRIVE:
    case PEDAL_EVENT_INPUT_POWER_STATE_CHARGE:
    case PEDAL_EVENT_INPUT_POWER_STATE_FAULT:
      s_brake_state.input_state.power_state = e->id == PEDAL_EVENT_INPUT_POWER_STATE_DRIVE;
      break;

    case PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED:
    case PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_RELEASED:
      s_brake_state.input_state.mech_brake = e->id == PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED;
      break;

    case PEDAL_EVENT_INPUT_DIRECTION_STATE_NEUTRAL:
    case PEDAL_EVENT_INPUT_DIRECTION_STATE_FORWARD:
    case PEDAL_EVENT_INPUT_DIRECTION_STATE_REVERSE:
      s_brake_state.input_state.direction = e->id != PEDAL_EVENT_INPUT_DIRECTION_STATE_NEUTRAL;
      break;

    case PEDAL_EVENT_INPUT_PEDAL_ACCEL:
    case PEDAL_EVENT_INPUT_PEDAL_COAST:
    case PEDAL_EVENT_INPUT_PEDAL_BRAKE:
      s_brake_state.input_state.throttle = e->id == PEDAL_EVENT_INPUT_PEDAL_BRAKE;
      break;

    case PEDAL_EVENT_INPUT_SPEED_UPDATE:
      s_brake_state.input_state.speed = e->data >= BRAKE_SIGNAL_MIN_SPEED_CMS;
      break;

    default:
      processed = false;
  }

  // (Power == Drive) &&
  //  (Mech Brake == Engaged ||
  //   (Direction != Neutral && Throttle == Brake && Speed > x))
  bool signal_on = s_brake_state.input_state.power_state &&
                   (s_brake_state.input_state.mech_brake ||
                    (s_brake_state.input_state.direction && s_brake_state.input_state.throttle &&
                     s_brake_state.input_state.speed));
  if (signal_on != s_brake_state.prev_signal_state) {
    s_brake_state.prev_signal_state = signal_on;

    CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_BRAKES, signal_on);
  }

  return processed;
}
