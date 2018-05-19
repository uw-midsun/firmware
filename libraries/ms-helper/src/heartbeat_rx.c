#include "heartbeat_rx.h"

#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_rx.h"
#include "status.h"

// CANRxHandlerCb
static StatusCode prv_heartbeat_handler(const CANMessage *msg, void *context,
                                        CANAckStatus *ack_reply) {
  HeartbeatRxHandlerStorage *storage = context;
  if (storage->handler != NULL && !storage->handler(msg->msg_id, storage->context)) {
    *ack_reply = CAN_ACK_STATUS_INVALID;
  }
  return STATUS_CODE_OK;
}

StatusCode heartbeat_rx_register_handler(HeartbeatRxHandlerStorage *storage, CANMessageID msg_id,
                                         HeartbeatRxHandler handler, void *context) {
  storage->handler = handler;
  storage->context = context;
  return can_register_rx_handler(msg_id, prv_heartbeat_handler, storage);
}

bool heartbeat_rx_auto_ack(CANMessageID msg_id, void *context) {
  (void)msg_id;
  (void)context;
  return true;
}
