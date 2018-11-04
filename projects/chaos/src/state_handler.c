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

static uint16_t s_sequence_lookup[NUM_EE_POWER_STATES] = {
  [EE_POWER_STATE_IDLE] = CHAOS_EVENT_SEQUENCE_IDLE,
  [EE_POWER_STATE_CHARGE] = CHAOS_EVENT_SEQUENCE_CHARGE,
  [EE_POWER_STATE_DRIVE] = CHAOS_EVENT_SEQUENCE_DRIVE,
};

// CanRxHandlerCb
static StatusCode prv_handle_power_state_msg(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  (void)context;
  EEPowerState power_state = NUM_EE_POWER_STATES;
  CAN_UNPACK_POWER_STATE(msg, (uint8_t *)&power_state);

  if (power_state >= NUM_EE_POWER_STATES) {
    *ack_reply = CAN_ACK_STATUS_INVALID;
  } else {
    if (!status_ok(event_raise_priority(EVENT_PRIORITY_HIGH, s_sequence_lookup[power_state], 0))) {
      // Raise an error if the event queue won't process the event.
      *ack_reply = CAN_ACK_STATUS_INVALID;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode state_handler_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_handle_power_state_msg, NULL);
}
