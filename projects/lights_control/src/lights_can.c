#include "can.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "status.h"

#include "exported_enums.h"
#include "lights_can.h"
#include "lights_events.h"

// RX handler for signal states messages.
static StatusCode prv_rx_signal_handler(const CANMessage *msg, void *context,
                                        CANAckStatus *ack_reply) {
  uint8_t signal_type = 0;
  uint8_t state = 0;
  status_ok_or_return(CAN_UNPACK_SIGNAL_LIGHT_STATE(msg, &signal_type, &state));
  if (signal_type >= NUM_EE_SIGNAL_LIGHT_TYPES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Unsupported signals type.");
  }
  return event_raise((state) ? LIGHTS_EVENT_SIGNAL_ON : LIGHTS_EVENT_SIGNAL_OFF,
                     (LightsEventSignalMode)signal_type);
}

// RX handler for lights states messages.
static StatusCode prv_rx_lights_handler(const CANMessage *msg, void *context,
                                        CANAckStatus *ack_reply) {
  LightsCanSettings *settings = (LightsCanSettings *)context;
  uint8_t light_type = 0;
  uint8_t state = 0;
  status_ok_or_return(CAN_UNPACK_LIGHT_STATE(msg, &light_type, &state));
  if (light_type >= NUM_EE_LIGHT_TYPES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Unsupported light type.");
  }
  return event_raise((state) ? LIGHTS_EVENT_GPIO_ON : LIGHTS_EVENT_GPIO_OFF,
                     (LightsEventSignalMode)settings->peripheral_lookup[light_type]);
}

// RX handler for lights sync messages.
static StatusCode prv_rx_sync_handler(const CANMessage *msg, void *context,
                                      CANAckStatus *ack_reply) {
  status_ok_or_return(CAN_UNPACK_LIGHTS_SYNC(msg));
  return event_raise(LIGHTS_EVENT_SYNC, 0);
}

// RX handler for BPS heartbeat.
static StatusCode prv_rx_bps_heartbeat_handler(const CANMessage *msg, void *context,
                                               CANAckStatus *ack_reply) {
  uint8_t status = 0;
  status_ok_or_return(CAN_UNPACK_BPS_HEARTBEAT(msg, &status));
  return event_raise(LIGHTS_EVENT_BPS_HEARTBEAT, status);
}

// RX handler for horn messages.
static StatusCode prv_rx_horn_handler(const CANMessage *msg, void *context,
                                      CANAckStatus *ack_reply) {
  uint8_t state = 0;
  status_ok_or_return(CAN_UNPACK_HORN(msg, &state));
  return event_raise((state) ? LIGHTS_EVENT_GPIO_ON : LIGHTS_EVENT_GPIO_OFF,
                     LIGHTS_EVENT_GPIO_PERIPHERAL_HORN);
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
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SIGNAL_LIGHT_STATE, prv_rx_signal_handler, NULL));
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHT_STATE, prv_rx_lights_handler,
                                              (void *)settings));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_SYNC, prv_rx_sync_handler, NULL));
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT,
                                              prv_rx_bps_heartbeat_handler, NULL));
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_HORN, prv_rx_horn_handler, NULL));
  return STATUS_CODE_OK;
}
