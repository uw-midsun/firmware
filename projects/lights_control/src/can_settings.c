#include <stdio.h>

#include "can_settings.h"
#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_unpack_impl.h"
#include "event_queue.h"
#include "status.h"
#include "gpio_addresses.h"


#define CASE_PERIPH(device) \
  case PERIPH_##device: \
    event_raise(EVENT_##device, *data); \
    break;

#define CAN_TX_ADDR { \
  SOMEPORT, \
  SOMEPIN \
}

#define CAN_RX_ADDR { \
  SOMEPORT, \
  SOMEPIN \
}

BoardType s_boardtype;
void * s_context;

static CANSettings s_can_settings = {
  .bitrate = CAN_HW_BITRATE_500KBPS, // TODO: need to figure out
  .rx_event = EVENT_CAN_RX,
  .tx_event = EVENT_CAN_TX,
  .fault_event = EVENT_CAN_FAULT,
  .tx = CAN_TX_ADDR,
  .rx = CAN_RX_ADDR
};

static StatusCode s_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // unpacks the message, raises events based on which board is selected
  uint8_t * PeripheralID = 0;
  uint8_t * data = 0;

  can_unpack_impl_u8(msg, msg->dlc, PeripheralID, data, NULL, NULL, NULL, NULL, NULL, NULL);

  printf("pripheral ID: %s", PeripheralID);
  printf("data: %s", data);

  if (s_boardtype == LIGHTS_BOARD_FRONT) {
    switch (*PeripheralID) {
      CASE_PERIPH(SIGNAL_RIGHT)
      CASE_PERIPH(SIGNAL_LEFT)
      CASE_PERIPH(SIGNAL_HAZARD)
      CASE_PERIPH(HORN)
      CASE_PERIPH(HEADLIGHTS)
    }
  } else if (s_boardtype == LIGHTS_BOARD_REAR) {
    switch (*PeripheralID) {
      CASE_PERIPH(SIGNAL_RIGHT)
      CASE_PERIPH(SIGNAL_LEFT)
      CASE_PERIPH(SIGNAL_HAZARD)
      CASE_PERIPH(BRAKE)
      CASE_PERIPH(STROBE)
    }
  }
  return STATUS_CODE_OK;
}

static CANMessageID s_msg_id = 0; 
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CAN_NUM_RX_HANDLERS];

StatusCode initialize_can_settings(BoardType boardtype) {
  if (boardtype == LIGHTS_BOARD_FRONT) {
    s_can_settings.device_id = CAN_DEVICE_LIGHTS_FRONT;
  } else if (boardtype == LIGHTS_BOARD_REAR) {
    s_can_settings.device_id = CAN_DEVICE_LIGHTS_REAR;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_boardtype = boardtype;

  // initialize CAN
  can_init(&s_can_settings, &s_can_storage, s_rx_handlers, CAN_NUM_RX_HANDLERS);
  can_register_rx_handler(s_msg_id, s_rx_handler, s_context);
  return STATUS_CODE_OK;
}

