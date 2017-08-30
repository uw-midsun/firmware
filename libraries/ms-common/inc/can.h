#pragma once
// CAN application interface
// Requires GPIO, soft timers, event queue, and interrupts to be initialized.
//
// Application code should only use functions in this header.
#include <stdbool.h>
#include <stdint.h>
#include "can_ack.h"
#include "can_fifo.h"
#include "can_hw.h"
#include "can_rx.h"
#include "fsm.h"
#include "gpio.h"

#define CAN_FSM can_get_fsm()

typedef struct CANSettings {
  uint16_t device_id;
  CANHwBitrate bitrate;
  GPIOAddress tx;
  GPIOAddress rx;
  EventID rx_event;
  EventID tx_event;
  EventID fault_event;
  bool loopback;
} CANSettings;

typedef struct CANStorage {
  FSM fsm;
  volatile CANFifo tx_fifo;
  volatile CANFifo rx_fifo;
  CANAckRequests ack_requests;
  CANRxHandlers rx_handlers;
  EventID rx_event;
  EventID tx_event;
  EventID fault_event;
  uint16_t device_id;
} CANStorage;

// Initializes the specified CAN configuration given pointers to persistant storage.
StatusCode can_init(const CANSettings *settings, CANStorage *storage, CANRxHandler *rx_handlers,
                    size_t num_rx_handlers);

StatusCode can_add_filter(CANMessageID msg_id);

StatusCode can_register_rx_default_handler(CANRxHandlerCb handler, void *context);

StatusCode can_register_rx_handler(CANMessageID msg_id, CANRxHandlerCb handler, void *context);

// Attempts to transmit the CAN message as soon as possible.
StatusCode can_transmit(const CANMessage *msg, const CANAckRequest *ack_request);

// Returns the FSM responsible for handling CAN messages. Use with fsm_process_event.
FSM *can_get_fsm(void);
