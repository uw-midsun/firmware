#include "state_handler.h"

#include <stddef.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "event_queue.h"

#define STATE_HANDLER_EMPTY_DATA 0

// CANRxHandlerCb
static StatusCode prv_handle_power_state_msg(const CANMessage *msg, void *context,
                                             CANAckStatus *ack_reply) {
  (void)context;
  uint8_t power_state = 0;
  CAN_UNPACK_POWER_STATE(msg, &power_state);
  // Handler the power state.
  // TODO(ELEC-105): Replace dummy values with those exported by driver controls.
  switch (power_state) {
    case 0:
      event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, STATE_HANDLER_EMPTY_DATA);
      break;
    case 1:
      event_raise(CHAOS_EVENT_SEQUENCE_IDLE, STATE_HANDLER_EMPTY_DATA);
      break;
    case 2:
      event_raise(CHAOS_EVENT_SEQUENCE_CHARGE, STATE_HANDLER_EMPTY_DATA);
      break;
    case 3:
      event_raise(CHAOS_EVENT_SEQUENCE_DRIVE, STATE_HANDLER_EMPTY_DATA);
      break;
    default:
      // Raise an error if the power state is invalid.
      *ack_reply = CAN_ACK_STATUS_INVALID;
  }
  return STATUS_CODE_OK;
}

StatusCode state_handler_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_handle_power_state_msg, NULL);
}
