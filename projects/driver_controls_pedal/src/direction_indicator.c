#include "direction_indicator.h"
#include "can_unpack.h"
#include "can_msg_defs.h"
#include "sc_input_event.h"

static StatusCode prv_handle_direction(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  uint16_t direction = 0;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &direction);

  switch(direction) {
    case EE:
      break;
    default:
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode direction_indicator_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CONSOLE_OUTPUT, prv_handle_direction, NULL);
  return STATUS_CODE_OK;
}
