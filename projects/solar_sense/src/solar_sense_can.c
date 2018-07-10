#include "solar_sense_can.h"

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // Storage for extracting message data.
  uint8_t relay_state = 0;
  switch (msg->msg_id) {
    case SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR:
    case SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT:
      status_ok_or_return(CAN_UNPACK_SOLAR_RELAY_REAR(msg, &relay_state));
      return event_raise(SOLAR_SENSE_EVENT_RELAY_STATE, relay_state);
  }


}

StatusCode solar_sense_can_init(CANStorage *storage,
                const CANSettings *can_settings) {

  status_ok_or_return(can_init(&storage->can_storage, can_settings));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR, prv_rx_handler, settings));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, prv_rx_handler, settings));
  return STATUS_CODE_OK;
}
