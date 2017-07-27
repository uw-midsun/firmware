#include "can_rx_fsm.h"
#include "can_rx.h"
#include "can.h"

FSM_DECLARE_STATE(can_rx_fsm_handle);

FSM_STATE_TRANSITION(can_rx_fsm_handle) {
  CANStorage *can_storage = fsm->context;
  FSM_ADD_TRANSITION(can_storage->rx_event, can_rx_fsm_handle);
}

static StatusCode prv_handle_data_msg(CANStorage *can_storage, const CANMessage *rx_msg) {
  CANRxHandler *handler = can_rx_get_handler(&can_storage->rx_handlers, rx_msg->msg_id);
  CANAckStatus ack_status = CAN_ACK_STATUS_OK;
  StatusCode ret = STATUS_CODE_OK;

  if (handler != NULL) {
    ret = handler->callback(rx_msg, handler->context, &ack_status);
  }

  if (CAN_MSG_IS_CRITICAL(rx_msg)) {
    CANMessage ack = {
      .msg_id = rx_msg->msg_id,
      .type = CAN_MSG_TYPE_ACK,
      .dlc = sizeof(ack_status),
      .data = ack_status
    };

    ret = can_transmit(&ack, NULL);
    status_ok_or_return(ret);
  }

  return ret;
}

static void prv_handle_rx(FSM *fsm, const Event *e, void *context) {
  CANStorage *can_storage = context;
  CANMessage rx_msg = { 0 };

  // Only process the number of messages that we popped in the corresponding event

  uint16_t messages = e->data;

  // printf("RX handling %d\n", e->data);
  while (messages--) {
    StatusCode result = can_queue_pop(&can_storage->rx_queue, &rx_msg);
    if (result != STATUS_CODE_OK) {
      // We had a mismatch between number of events and number of messages, so return silently
      // Alternatively, we could use the data value of the event.
      return;
    }

    // We currently ignore failures to handle the message.
    // If needed, we could push it back to the queue.
    switch (rx_msg.type) {
      case CAN_MSG_TYPE_ACK:
        result = can_ack_handle_msg(&can_storage->ack_requests, &rx_msg);

        break;
      case CAN_MSG_TYPE_DATA:
        result = prv_handle_data_msg(can_storage, &rx_msg);

        break;
      default:
        status_msg(STATUS_CODE_UNREACHABLE, "CAN RX: Invalid type");

        return;
        // error
        break;
    }
  }
}

StatusCode can_rx_fsm_init(FSM *rx_fsm, CANStorage *can_storage) {
  if (rx_fsm == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  fsm_init(rx_fsm, "can_rx_fsm", &can_rx_fsm_handle, can_storage);
  fsm_state_init(can_rx_fsm_handle, prv_handle_rx);

  return STATUS_CODE_OK;
}
