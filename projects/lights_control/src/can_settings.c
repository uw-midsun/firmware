#include <stdio.h>

#include "can_settings.h"
#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_unpack_impl.h"
#include "event_queue.h"
#include "status.h"

#define SOMEPORT 0
#define SOMEPIN 0

#define CASE_ACTION(action) \
  case ACTION_##action: \
    event_raise(EVENT_##action, *data); \
    break;

#define CAN_RX_ADDR { \
  0, \
  11\
}

#define CAN_TX_ADDR { \
  0, \
  12\
}

#define CAN_DEVICE_ID_FRONT 0
#define CAN_DEVICE_ID_REAR  0

typedef enum {
  ACTION_SIGNAL_RIGHT,
  ACTION_SIGNAL_LEFT,
  ACTION_SIGNAL_HAZARD,
  ACTION_HORN,
  ACTION_HEADLIGHTS,
  ACTION_BRAKES,
  ACTION_STROBE,
  NUM_ACTION_ID
} ActionID;

BoardType s_boardtype;
volatile CANMessage s_rx_msg = { 0 };

static CANSettings s_can_settings = {
  .bitrate = CAN_HW_BITRATE_125KBPS, // TODO: need to figure out
  .rx_event = EVENT_CAN_RX,
  .tx_event = EVENT_CAN_TX,
  .fault_event = EVENT_CAN_FAULT,
  .tx = CAN_TX_ADDR,
  .rx = CAN_RX_ADDR,
  .loopback = true
};

static CANMessageID s_msg_id = 0x1; 
static CANStorage s_can_storage;
static CANRxHandler prv_rx_handlers[CAN_NUM_RX_HANDLERS];
static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply);

StatusCode initialize_can_settings(BoardType boardtype) {
  if (boardtype == LIGHTS_BOARD_FRONT) {
    s_can_settings.device_id = CAN_DEVICE_ID_FRONT;
  } else if (boardtype == LIGHTS_BOARD_REAR) {
    s_can_settings.device_id = CAN_DEVICE_ID_REAR;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_boardtype = boardtype;

  // initialize CAN
  can_init(&s_can_settings, &s_can_storage, prv_rx_handlers, CAN_NUM_RX_HANDLERS);

  can_register_rx_handler(s_msg_id, prv_rx_handler, &s_rx_msg);
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  printf("CAN MESSAGE RECEIVED!");
  // unpacks the message, raises events based on which board is selected
  uint8_t * PeripheralID = 0;
  uint8_t * data = 0;

  can_unpack_impl_u8(msg, msg->dlc, PeripheralID, data, NULL, NULL, NULL, NULL, NULL, NULL);

  printf("pripheral ID: %s", PeripheralID);
  printf("data: %s", data);

  if (s_boardtype == LIGHTS_BOARD_FRONT) {
    switch (*PeripheralID) {
      CASE_ACTION(SIGNAL_RIGHT)
      CASE_ACTION(SIGNAL_LEFT)
      CASE_ACTION(SIGNAL_HAZARD)
      CASE_ACTION(HORN)
      CASE_ACTION(HEADLIGHTS)
      default:
        return STATUS_CODE_INVALID_ARGS;
    }
  } else if (s_boardtype == LIGHTS_BOARD_REAR) {
    switch (*PeripheralID) {
      CASE_ACTION(SIGNAL_RIGHT)
      CASE_ACTION(SIGNAL_LEFT)
      CASE_ACTION(SIGNAL_HAZARD)
      CASE_ACTION(BRAKES)
      CASE_ACTION(STROBE)
      default:
        return STATUS_CODE_INVALID_ARGS;
    }
  }
  return STATUS_CODE_OK;
}

