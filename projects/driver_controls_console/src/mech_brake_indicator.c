#include "mech_brake_indicator.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "cc_input_event.h"

static StatusCode prv_handle_mech_brake(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  uint16_t throttle = 0;
  uint16_t mech_brake_pressed = 0;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle, &mech_brake_pressed);

  if (mech_brake_pressed) {
    event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CENTER_CONSOLE_MECH_BRAKE_PRESSED,
                         mech_brake_pressed);
  } else {
    event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_CENTER_CONSOLE_MECH_BRAKE_RELEASED,
                         mech_brake_pressed);
  }

  return STATUS_CODE_OK;
}

StatusCode mech_brake_indicator_init(void) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_mech_brake, NULL);
  return STATUS_CODE_OK;
}
