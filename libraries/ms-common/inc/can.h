#pragma once
// CAN application interface
//
// Application code should only use functions in this header.
#include <stdint.h>
#include <stdbool.h>
#include "can_ack.h"
#include "can_hw.h"
#include "can_queue.h"
#include "can_rx.h"
#include "fsm.h"

typedef struct CANConfig {
  FSM fsm;
  CANHwConfig hw;
  uint16_t device_id;
  EventID rx_event;
  EventID fault_event;
  CANQueue tx_queue;
  CANQueue rx_queue;
  CANAckRequests ack_requests;
  CANRxHandlers rx_handlers;
} CANConfig;

// Initializes the specified CAN configuration given a pointer to CAN config storage
StatusCode can_init(CANConfig *can, uint16_t device_id, uint16_t bus_speed, bool loopback,
                    EventID rx_event, EventID fault_event);

StatusCode can_add_filter(CANConfig *can, CANMessageID msg_id);

StatusCode can_register_rx_handler(CANConfig *can, CANMessageID msg_id,
                                   CANRxHandlerCb handler, void *context);

StatusCode can_transmit(CANConfig *can, const CANMessage *msg, const CANAckRequest *ack_request);
