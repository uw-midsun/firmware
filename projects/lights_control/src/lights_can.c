#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "status.h"

#include "exported_enums.h"
#include "lights_can.h"
#include "lights_events.h"

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  LightsCanSettings *settings = (LightsCanSettings *)context;
  // Storage for extracting message data.
  uint8_t param_1 = 0;
  uint8_t param_2 = 0;
  switch (msg->msg_id) {
    case SYSTEM_CAN_MESSAGE_LIGHT_STATE:
      status_ok_or_return(CAN_UNPACK_LIGHT_STATE(msg, &param_1, &param_2));
      return event_raise(settings->event_id_lookup[param_1][param_2],
                         settings->event_data_lookup[param_1]);
    case SYSTEM_CAN_MESSAGE_HORN:
      status_ok_or_return(CAN_UNPACK_HORN(msg, &param_1));
      return event_raise((param_1) ? LIGHTS_EVENT_GPIO_ON : LIGHTS_EVENT_GPIO_OFF,
                         LIGHTS_EVENT_GPIO_PERIPHERAL_HORN);
    case SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT:
      status_ok_or_return(CAN_UNPACK_BPS_HEARTBEAT(msg, &param_1));
      return event_raise(LIGHTS_EVENT_BPS_HEARTBEAT, param_1);
    case SYSTEM_CAN_MESSAGE_LIGHTS_SYNC:
      return event_raise(LIGHTS_EVENT_SYNC, 0);
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid message id");
}

StatusCode lights_can_init(LightsCanStorage *storage, const LightsCanSettings *settings) {
  CANSettings can_settings = {
    .device_id = settings->device_id,
    .bitrate = settings->bitrate,
    .rx_event = LIGHTS_EVENT_CAN_RX,
    .tx_event = LIGHTS_EVENT_CAN_TX,
    .fault_event = LIGHTS_EVENT_CAN_FAULT,
    .tx = settings->tx_addr,
    .rx = settings->rx_addr,
    .loopback = settings->loopback,
  };
  // Initialize CAN.
  status_ok_or_return(can_init(&can_settings, &storage->can_storage, storage->rx_handlers,
                               LIGHTS_CAN_NUM_RX_HANDLERS));
  // Initialize CAN RX handlers.
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHT_STATE, prv_rx_handler, settings));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_SYNC, prv_rx_handler, settings));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_rx_handler, settings));
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_HORN, prv_rx_handler, settings));
  return STATUS_CODE_OK;
}
