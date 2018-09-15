#pragma once
// CAN application interface
// Requires GPIO, soft timers, event queue, and interrupts to be initialized.
//
// Application code should only use functions in this header.
// Note that the CAN FSM must be clocked. Call `can_process_event(&e)` in your event loop.
//
// See:
// * can_rx: CANRxHandlerCb
// * can_ack: CANAckRequest
#include <stdbool.h>
#include <stdint.h>
#include "can_ack.h"
#include "can_fifo.h"
#include "can_hw.h"
#include "can_rx.h"
#include "fsm.h"
#include "gpio.h"

#define CAN_NUM_RX_HANDLERS 10

typedef struct CANSettings {
  uint16_t device_id;
  CANHwBitrate bitrate;
  GpioAddress tx;
  GpioAddress rx;
  EventId rx_event;
  EventId tx_event;
  EventId fault_event;
  bool loopback;
} CANSettings;

typedef struct CANStorage {
  FSM fsm;
  volatile CanFifo tx_fifo;
  volatile CanFifo rx_fifo;
  CANAckRequests ack_requests;
  CANRxHandlers rx_handlers;
  CANRxHandler rx_handler_storage[CAN_NUM_RX_HANDLERS];
  EventId rx_event;
  EventId tx_event;
  EventId fault_event;
  uint16_t device_id;
} CANStorage;

// Initializes the specified CAN configuration.
StatusCode can_init(CANStorage *storage, const CANSettings *settings);

// Adds a hardware filter for the specified message ID.
StatusCode can_add_filter(CanMessageId msg_id);

// Registers a default RX handler for messages without specific RX handlers.
StatusCode can_register_rx_default_handler(CANRxHandlerCb handler, void *context);

// Registers an RX handler for a specific message ID.
StatusCode can_register_rx_handler(CanMessageId msg_id, CANRxHandlerCb handler, void *context);

// Attempts to transmit the CAN message as soon as possible.
StatusCode can_transmit(const CANMessage *msg, const CANAckRequest *ack_request);

// Processes the registered events. This must be called for the CAN network layer to work.
bool can_process_event(const Event *e);
