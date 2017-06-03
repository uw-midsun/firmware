#include "can.h"
#include "soft_timer.h"

// Attempts to transmit the specified message using the HW TX, overwriting the source device.
StatusCode prv_transmit(const CANConfig *can, const CANMessage *msg) {
  CANId msg_id = {
    .source_id = can->device_id,
    .type = msg->type,
    .msg_id = msg->msg_id
  };

  return can_hw_transmit(&can->hw, msg_id.raw, msg->data_u8, msg->dlc);
}

// Handler for CAN HW TX ready events
// Attempts to transmit any messages in the TX queue
void prv_tx_handler(void *context) {
  CANConfig *can = context;
  CANMessage tx_msg;

  while (can_queue_size(&can->tx_queue) > 0) {
    can_queue_peek(&can->tx_queue, &tx_msg);

    StatusCode result = prv_transmit(can, &tx_msg);

    if (result != STATUS_CODE_OK) {
      return;
    }

    can_queue_pop(&can->tx_queue, NULL);
  }
}

// Handler for CAN HW message RX events
// Dumps received messages to the RX queue and raises an event for the messages to be processed
void prv_rx_handler(void *context) {
  CANConfig *can = context;
  CANId rx_id;
  CANMessage rx_msg;
  size_t counter = 0;

  while (can_hw_receive(&can->hw, &rx_id.raw, &rx_msg.data, &rx_msg.dlc)) {
    CAN_MSG_SET_RAW_ID(&rx_msg, rx_id.raw);

    StatusCode result = can_queue_push(&can->rx_queue, &rx_msg);
    if (result != STATUS_CODE_OK) {
      return;
    }

    counter++;
  }

  if (counter != 0) {
    event_raise(can->rx_event, counter);
  }
}

// Bus error timer callback
// Checks if the bus has recovered, raising the fault event if still off
void prv_bus_error_timeout_handler(SoftTimerID timer_id, void *context) {
  CANConfig *can = context;

  if (can_hw_bus_status(&can->hw) == CAN_HW_BUS_STATUS_OFF) {
    event_raise(can->fault_event, 0);
  }
}

// Handler for CAN HW bus error events
// Starts a timer to check for bus recovery
void prv_bus_error_handler(void *context) {
  CANConfig *can = context;

  // TODO: replace magic number
  soft_timer_start_seconds(1, prv_bus_error_timeout_handler, can, NULL);
}

StatusCode can_init(CANConfig *can, uint16_t device_id, uint16_t bus_speed, bool loopback,
                    EventID rx_event, EventID fault_event) {
  // TODO: Add checks

  memset(can, 0, sizeof(*can));

  can_hw_init(&can->hw, bus_speed, loopback);
  can->device_id = device_id;
  can->rx_event = rx_event;
  can->fault_event = fault_event;

  can_hw_register_callback(&can->hw, CAN_HW_EVENT_TX_READY, prv_tx_handler, can);
  can_hw_register_callback(&can->hw, CAN_HW_EVENT_MSG_RX, prv_rx_handler, can);
  can_hw_register_callback(&can->hw, CAN_HW_EVENT_BUS_ERROR, prv_bus_error_handler, can);

  return STATUS_CODE_OK;
}

StatusCode can_transmit(CANConfig *can, const CANMessage *msg, const CANAckRequest *ack_request) {
  // TODO: Add check for valid critical message if ack requested

  if (ack_request != NULL) {
    // TODO: figure out what to do here
    // status_ok_or_return(can_ack_add_request(&can->ack_requests, msg->msg_id, ack_request));
  }

  // We could push the message first and attempt to transmit the message with the lowest ID,
  // but there really isn't any point. We get an interrupt whenever a TX mailbox is empty and
  // attempt to fill it, so that should usually take precedence over this function.
  StatusCode tx_result = prv_transmit(&can, msg);
  if (tx_result != STATUS_CODE_OK) {
    // As long as the timer for the ACK request is started, it's okay for the message to fail.
    // An optimization would be to fail early and signal that as an error.
    return can_queue_push(&can->tx_queue, msg);
  }

  return STATUS_CODE_OK;
}
