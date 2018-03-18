#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "status.h"

#include "lights_can.h"
#include "lights_events.h"

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CAN_NUM_RX_HANDLERS];

static StatusCode prv_rx_handler_front(const CANMessage *msg, void *context,
                                       CANAckStatus *ack_reply);

static StatusCode prv_rx_handler_rear(const CANMessage *msg, void *context,
                                      CANAckStatus *ack_reply);

static LightsEvent s_event_lookup[NUM_ACTION_ID] = {
  [LIGHTS_ACTION_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_RIGHT,    //
  [LIGHTS_ACTION_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_LEFT,      //
  [LIGHTS_ACTION_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_HAZARD,  //
  [LIGHTS_ACTION_HORN] = LIGHTS_EVENT_HORN,                    //
  [LIGHTS_ACTION_HEADLIGHTS] = LIGHTS_EVENT_HEADLIGHTS,        //
  [LIGHTS_ACTION_BRAKES] = LIGHTS_EVENT_BRAKES,                //
  [LIGHTS_ACTION_STROBE] = LIGHTS_EVENT_STROBE,                //
  [LIGHTS_ACTION_SYNC] = LIGHTS_EVENT_SYNC,                    //
};

StatusCode lights_can_init(const CANSettings *can_settings) {
  CANMessageID msg_id = SYSTEM_CAN_MESSAGE_LIGHTS_STATES;
  StatusCode can_init_status, can_register_rx_status;
  // initialize CAN
  can_init_status = can_init(can_settings, &s_can_storage, s_rx_handlers, CAN_NUM_RX_HANDLERS);
  if (can_init_status != STATUS_CODE_OK) {
    return can_init_status;
  }
  if (can_settings->device_id == SYSTEM_CAN_DEVICE_LIGHTS_FRONT) {
    can_register_rx_status = can_register_rx_handler(msg_id, prv_rx_handler_front, NULL);
  } else {
    can_register_rx_status = can_register_rx_handler(msg_id, prv_rx_handler_rear, NULL);
  }
  if (!can_register_rx_status != STATUS_CODE_OK) {
    return can_register_rx_status;
  }
  return STATUS_CODE_OK;
}

// RX handler for front board.
static StatusCode prv_rx_handler_front(const CANMessage *msg, void *context,
                                       CANAckStatus *ack_reply) {
  // unpacks the message, raises events based on which board is selected
  uint8_t action_id = 0;
  uint8_t data = 0;

  if (action_id >= NUM_ACTION_ID) {
    return STATUS_CODE_INVALID_ARGS;
  }

  CAN_UNPACK_LIGHTS_STATES(msg, &action_id, &data);
  event_raise(s_event_lookup[action_id], data);
  return STATUS_CODE_OK;
}

// RX handler for rear board.
static StatusCode prv_rx_handler_rear(const CANMessage *msg, void *context,
                                      CANAckStatus *ack_reply) {
  // unpacks the message, raises events based on which board is selected
  uint8_t action_id = 0;
  uint8_t data = 0;
  if (action_id >= NUM_ACTION_ID) {
    return STATUS_CODE_INVALID_ARGS;
  }

  CAN_UNPACK_LIGHTS_STATES(msg, &action_id, &data);
  event_raise(s_event_lookup[action_id], data);
  return STATUS_CODE_OK;
}
