#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"

#include "can_unpack_impl.h"
#include "event_queue.h"
#include "status.h"

#include "lights_can.h"
#include "lights_events.h"
#include "lights_gpio.h"

#define SOMEPORT 0
#define SOMEPIN 0

#define CASE_ACTION(action)            \
  case ACTION_##action:                \
    event_raise(EVENT_##action, data); \
    break;

#define CAN_RX_ADDR \
  { 0, 11 }

#define CAN_TX_ADDR \
  { 0, 12 }

// TODO(ELEC-165): remove these and get the device ID from elsewhere
#define CAN_DEVICE_ID_FRONT 0
#define CAN_DEVICE_ID_REAR 0

static BoardType s_boardtype;

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CAN_NUM_RX_HANDLERS];

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply);

// TODO(ELEC-165):
// need to figure out:
//      1.bitrate for can_settings
//      2.CAN_TX_ADDR
//      3.CAN_RX_ADDR
//      4.msg_id
void lights_can_init(BoardType boardtype, bool loopback) {
  CANSettings can_settings = { .bitrate = CAN_HW_BITRATE_125KBPS,
                               .rx_event = EVENT_CAN_RX,
                               .tx_event = EVENT_CAN_TX,
                               .fault_event = EVENT_CAN_FAULT,
                               .tx = CAN_TX_ADDR,
                               .rx = CAN_RX_ADDR,
                               .loopback = loopback };
  volatile CANMessage rx_msg = { 0 };
  CANMessageID msg_id = 0x1;
  s_boardtype = boardtype;
  can_settings.device_id =
      (boardtype == LIGHTS_BOARD_FRONT) ? CAN_DEVICE_ID_FRONT : CAN_DEVICE_ID_REAR;

  // initialize CAN
  can_init(&can_settings, &s_can_storage, s_rx_handlers, CAN_NUM_RX_HANDLERS);
  can_register_rx_handler(msg_id, prv_rx_handler, &rx_msg);
}

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // unpacks the message, raises events based on which board is selected
  uint8_t peripheral_id = 0;
  uint8_t data = 0;

  can_unpack_impl_u8(msg, msg->dlc, &peripheral_id, &data, NULL, NULL, NULL, NULL, NULL, NULL);

  if (s_boardtype == LIGHTS_BOARD_FRONT) {
    switch (peripheral_id) {
      CASE_ACTION(SIGNAL_RIGHT)
      CASE_ACTION(SIGNAL_LEFT)
      CASE_ACTION(SIGNAL_HAZARD)
      CASE_ACTION(HORN)
      CASE_ACTION(HEADLIGHTS)
      CASE_ACTION(SYNC)
    }
  } else if (s_boardtype == LIGHTS_BOARD_REAR) {
    switch (peripheral_id) {
      CASE_ACTION(SIGNAL_RIGHT)
      CASE_ACTION(SIGNAL_LEFT)
      CASE_ACTION(SIGNAL_HAZARD)
      CASE_ACTION(BRAKES)
      CASE_ACTION(STROBE)
      CASE_ACTION(SYNC)
    }
  }
  return STATUS_CODE_OK;
}

// sends sync message:
// both boards receive sync and reset the blinker
// TODO(ELEC-165) see if the lights boards necessarily have
// to use different message id's, cuz it'd be cool if
// they didn't. That way they both process the sync
// message the same way, one uses loopback.
StatusCode send_sync(void) {
  CANMessage msg = {
    .msg_id = 0x1,              //
    .type = CAN_MSG_TYPE_DATA,  //
    .data = 0x107,              //
    .dlc = 2,                   //
  };

  event_raise(EVENT_SYNC, 1);
  return can_transmit(&msg, NULL);
}
