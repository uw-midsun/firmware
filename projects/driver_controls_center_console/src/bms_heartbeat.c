#include "bms_heartbeat.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "input_event.h"
#include "log.h"
#include "status.h"

// CanRxHandlerCb
static StatusCode prv_bms_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  uint8_t state = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &state);

  if (state != EE_BPS_HEARTBEAT_STATE_OK) {
    LOG_DEBUG("Emergency: BPS Fault\n");

    // We need to cause a transition in the LED FSM
    event_raise(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, EE_CENTER_CONSOLE_DIGITAL_INPUT_BPS);
  }

  return STATUS_CODE_OK;
}

StatusCode bms_heartbeat_init(void) {
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_bms_rx, NULL));

  return STATUS_CODE_OK;
}
