#include "fault_handler.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "input_event.h"

static StatusCode prv_handle_heartbeat(const CANMessage *msg, void *context,
                                       CANAckStatus *ack_reply) {
  FaultHandlerStorage *storage = context;

  uint8_t data = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &data);

  EEBpsHeartbeatState state = data;
  if (state != EE_BPS_HEARTBEAT_STATE_OK) {
    event_raise_priority(EVENT_PRIORITY_HIGHEST, INPUT_EVENT_BPS_FAULT, 0);
    CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_STROBE, EE_LIGHT_STATE_ON);
  }

  return STATUS_CODE_OK;
}

StatusCode fault_handler_init(FaultHandlerStorage *storage) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_handle_heartbeat, storage);
}

StatusCode fault_handler_clear_fault(FaultHandlerStorage *storage) {
  return CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_STROBE, EE_LIGHT_STATE_OFF);
}
