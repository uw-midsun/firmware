#pragma once
// RX handler for heartbeats.
//
// Requires CAN to be initialized.
//
// The concept is to simplify registering a CANRxHandler such that it automatically replies in the
// affirmative to the message unless a callback specifies otherwise. If no handler is specified
// |NULL| it will always return success.

#include <stdbool.h>

#include "can.h"
#include "status.h"

// Return true to respond in the affirmative. False to fail the ACK.
typedef bool (*HeartbeatRxHandler)(CANMessageID msg_id, void *context);

typedef struct HeartbeatRxHandlerStorage {
  HeartbeatRxHandler handler;
  void *context;
} HeartbeatRxHandlerStorage;

// Registers the heartbeat handler (|handler|) to run for |msg_id|. |handler| takes |context| as an
// argument. This configuration is stored in |storage| which must persist indefinitely. If |handler|
// is NULL the heartbeat will automatically be acknowledged in the affirmative.
StatusCode heartbeat_rx_register_handler(HeartbeatRxHandlerStorage *storage, CANMessageID msg_id,
                                         HeartbeatRxHandler handler, void *context);

// An instance of HeartbeatRxHandler that can be used to automatically ack and return true with no
// other behavior.
bool heartbeat_rx_auto_ack(CANMessageID msg_id, void *context);
