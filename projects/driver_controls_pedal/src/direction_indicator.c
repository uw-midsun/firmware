#include "direction_indicator.h"
#include "can.h"
#include "exported_enums.h"
#include "can_unpack.h"
#include "can_msg_defs.h"
#include "pc_input_event.h"

static StatusCode prv_handle_direction(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  uint16_t direction = 0;
  CAN_UNPACK_CONSOLE_OUTPUT(msg, &direction);

  switch(direction) {
    case EE_CONSOLE_OUTPUT_DIRECTION_NEUTRAL:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
        INPUT_EVENT_PEDAL_DIRECTION_STATE_NEUTRAL, 0);
      break;
    case EE_CONSOLE_OUTPUT_DIRECTION_FORWARD:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
        INPUT_EVENT_PEDAL_DIRECTION_STATE_FORWARD, 0);
      break;
    case EE_CONSOLE_OUTPUT_DIRECTION_REVERSE:
      event_raise_priority(EVENT_PRIORITY_NORMAL,
        INPUT_EVENT_PEDAL_DIRECTION_STATE_REVERSE, 0);
      break;

    // should not reach this point
    default:
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode direction_indicator_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CONSOLE_OUTPUT, prv_handle_direction, NULL);
  return STATUS_CODE_OK;
}
