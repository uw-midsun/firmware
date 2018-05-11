#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "status.h"

#include "lights_can.h"
#include "lights_events.h"

// RX handler function.
static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  const LightsCanSettings *settings = (const LightsCanSettings *)context;

  // Unpacks the message, and raises events.
  uint8_t action_id = 0;
  uint8_t data = 0;
  CAN_UNPACK_LIGHTS_STATES(msg, &action_id, &data);
  if (action_id >= NUM_LIGHTS_CAN_ACTIONS_ID) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Unsupported lights action.");
  }
  status_ok_or_return(event_raise(settings->event_lookup[action_id], data));
  return STATUS_CODE_OK;
}

StatusCode lights_can_init(const LightsCanSettings *settings, LightsCanStorage *storage) {
  CANMessageID msg_id = SYSTEM_CAN_MESSAGE_LIGHTS_STATES;
  CANSettings can_settings = {
    .device_id = settings->device_id,
    .bitrate = CAN_HW_BITRATE_125KBPS,
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

  // Initialize CAN RX handler.
  status_ok_or_return(can_register_rx_handler(msg_id, prv_rx_handler, (void *)settings));
  return STATUS_CODE_OK;
}
