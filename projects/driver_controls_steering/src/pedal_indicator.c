#include "pedal_indicator.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "sc_input_event.h"
#include "exported_enums.h"

static StatusCode prv_handle_mech_brake(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  uint16_t throttle = 0;
  uint16_t throttle_state = 0;
  uint16_t mech_brake_pressed = 0;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle, &throttle_state, &mech_brake_pressed);

  if (mech_brake_pressed) {
    event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_STEERING_MECHANICAL_BRAKE_PRESSED,
                         mech_brake_pressed);
  } else {
    event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_STEERING_MECHANICAL_BRAKE_RELEASED,
                         mech_brake_pressed);
  }

  switch(throttle_state) {
    case EE_THROTTLE_BRAKE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_STEERING_PEDAL_BRAKE, 0);
      break;

    case EE_THROTTLE_COAST:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_STEERING_PEDAL_COAST, 0);
      break;

    case EE_THROTTLE_ACCEL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_STEERING_PEDAL_ACCEL, 0);
      break;

    default:
      event_raise_priority(EVENT_PRIORITY_HIGHEST, INPUT_EVENT_STEERING_PEDAL_FAULT, 0);
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode mech_brake_indicator_init(void) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_mech_brake, NULL);
  return STATUS_CODE_OK;
}
