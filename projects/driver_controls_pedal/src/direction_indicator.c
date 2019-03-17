#include "direction_indicator.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "pc_input_event.h"

static StatusCode prv_handle_direction(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {

  int16_t pedal = 0, direction = 0, cruise = 0, mech_brake = 0;

  CAN_UNPACK_DRIVE_OUTPUT(msg, (uint16_t *)&pedal, (uint16_t *)&direction,
                                              (uint16_t *)&cruise, (uint16_t *)&mech_brake);

  switch (direction) {
    case EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_DIRECTION_STATE_NEUTRAL, 0);
      break;
    case EE_DRIVE_OUTPUT_DIRECTION_FORWARD:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_DIRECTION_STATE_FORWARD, 0);
      break;
    case EE_DRIVE_OUTPUT_DIRECTION_REVERSE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_DIRECTION_STATE_REVERSE, 0);
      break;

    // should not reach this point
    default:
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode direction_indicator_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_handle_direction, NULL);
  return STATUS_CODE_OK;
}
