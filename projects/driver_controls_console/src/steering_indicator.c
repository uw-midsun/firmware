#include "steering_indicator.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "cc_input_event.h"
#include "exported_enums.h"

static StatusCode prv_handle_steering(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  DriveOutputStorage *storage = context;

  uint16_t control_stalk_analog_state = 0;
  uint16_t control_stalk_digital_state = 0;

  CAN_UNPACK_STEERING_OUTPUT(msg, &control_stalk_analog_state, &control_stalk_digital_state);

  switch (control_stalk_analog_state) {
    case EE_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_DISTANCE_MINUS:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_MINUS,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_DISTANCE_PLUS:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_PLUS,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_SPEED_MINUS:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_SPEED_PLUS:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_DIGITAL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_DIGITAL, 0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_CANCEL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, 0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_RESUME:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, 0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_NONE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE,
                           0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_RIGHT:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT, 0);
      break;
    case EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_LEFT:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT,
                           0);
      break;
    default:
      break;
  }

  switch (control_stalk_digital_state) {
    case EE_CONTROL_STALK_DIGITAL_CC_SET_PRESSED:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED,
                           0);
      break;
    case EE_CONTROL_STALK_DIGITAL_CC_SET_RELEASED:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED,
                           0);
      break;
    case EE_CONTROL_STALK_DIGITAL_CC_OFF:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_CC_ON:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_ON, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_FWD_PRESSED:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_FWD_RELEASED:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_BACK_PRESSED:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_PRESSED, 0);
      break;
    case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_BACK_RELEASED:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
                           INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_RELEASED, 0);
      break;
    default:
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode steering_indicator_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT, prv_handle_steering,
                                 drive_output_global());
}
