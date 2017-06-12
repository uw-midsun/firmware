#pragma once
// CAN RX handlers
#include <stdint.h>
#include "status.h"
#include "can_msg.h"
#include "can_ack.h"

// Process the received CAN message. Storage to an ACK reply is valid only if the message
// is considered critical. If valid, the ack reply is OK by default.
typedef StatusCode (*CANRxHandlerCb)(const CANMessage *msg, void *context, CANAckStatus *ack_reply);

typedef struct CANRxHandler {
  CANRxHandlerCb callback;
  void *context;
  CANMessageID msg_id;
} CANRxHandler;

typedef struct CANRxHandlers {
  CANRxHandler *storage;
  size_t max_handlers;
  size_t num_handlers;
} CANRxHandlers;

StatusCode can_rx_init(CANRxHandlers *rx_handlers,
                       CANRxHandler *handler_storage, size_t num_handlers);

// TODO(ELEC-55): should we support a default handler?
StatusCode can_rx_register_handler(CANRxHandlers *rx_handlers, CANMessageID msg_id,
                                   CANRxHandlerCb handler, void *context);

CANRxHandler *can_rx_get_handler(CANRxHandlers *rx_handlers, CANMessageID msg_id);
