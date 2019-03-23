#include "power_state_indicator.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "pc_input_event.h"

static StatusCode prv_handle_power_state(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  uint8_t state = 0;
  CAN_UNPACK_POWER_STATE(msg, &state);

  switch (state) {
    case EE_POWER_STATE_IDLE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_POWER_STATE_OFF, 0);
      break;
    case EE_POWER_STATE_CHARGE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_POWER_STATE_CHARGE, 0);
      break;
    case EE_POWER_STATE_DRIVE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_POWER_STATE_DRIVE, 0);
      break;

    default:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_POWER_STATE_OFF, 0);
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode power_state_indicator_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_handle_power_state, NULL);
  return STATUS_CODE_OK;
}
