#include "solar_sense_can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "i2c.h"
#include "solar_sense_event.h"

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // Storage for extracting message data.
  uint8_t relay_state = 0;
  switch (msg->msg_id) {
    case SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR:
      status_ok_or_return(CAN_UNPACK_SOLAR_RELAY_REAR(msg, &relay_state));
      break;
    case SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT:
      status_ok_or_return(CAN_UNPACK_SOLAR_RELAY_FRONT(msg, &relay_state));
      break;
  }
  return event_raise(SOLAR_SENSE_EVENT_RELAY_STATE, relay_state);
}

StatusCode solar_sense_can_init(SolarSenseCanStorage *storage, const CANSettings *can_settings,
                                SolarSenseConfigBoard board) {
  status_ok_or_return(can_init(&storage->can_storage, can_settings));
  storage->board = board;
  // Specify filters. Since we add filters, the handlers don't need to care about board type.
  status_ok_or_return(can_add_filter((storage->board == SOLAR_SENSE_CONFIG_BOARD_FRONT)
                                         ? SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT
                                         : SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR));
  // Set up RX handlers.
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR, prv_rx_handler, NULL));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, prv_rx_handler, NULL));
  return STATUS_CODE_OK;
}
