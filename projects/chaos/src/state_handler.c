#include "state_handler.h"

#include <stddef.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "sequencer_fsm.h"

#define STATE_HANDLER_EMPTY_DATA 0

// CANRxHandlerCb
static StatusCode prv_handle_power_state_msg(const CANMessage *msg, void *context,
                                             CANAckStatus *ack_reply) {
  (void)context;
  EEPowerState power_state = NUM_EE_POWER_STATES;
  CAN_UNPACK_POWER_STATE(msg, (uint8_t *)&power_state);

  // Handle the power state.
  switch (power_state) {
    case EE_POWER_STATE_IDLE:
      if (!sequencer_fsm_enqueue(CHAOS_EVENT_SEQUENCE_IDLE)) {
        // Raise an error if the power state is invalid.
        *ack_reply = CAN_ACK_STATUS_INVALID;
      }
      break;
    case EE_POWER_STATE_CHARGE:
      if (!sequencer_fsm_enqueue(CHAOS_EVENT_SEQUENCE_CHARGE)) {
        // Raise an error if the power state is invalid.
        *ack_reply = CAN_ACK_STATUS_INVALID;
      }
      break;
    case EE_POWER_STATE_DRIVE:
      if (!sequencer_fsm_enqueue(CHAOS_EVENT_SEQUENCE_DRIVE)) {
        // Raise an error if the power state is invalid.
        *ack_reply = CAN_ACK_STATUS_INVALID;
      }
      break;
    case NUM_EE_POWER_STATES:  // Falls through.
    default:
      // Raise an error if the power state is invalid.
      *ack_reply = CAN_ACK_STATUS_INVALID;
  }
  return STATUS_CODE_OK;
}

StatusCode state_handler_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_handle_power_state_msg, NULL);
}
