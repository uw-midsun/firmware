#include "can_rx_fsm.h"
#include "can_rx.h"
#include "can.h"

#define CAN_ACK_EXPECTED(msg) ((msg)->msg_id < 14)

FSM_DECLARE_STATE(can_rx_fsm_handle);

FSM_STATE_TRANSITION(can_rx_fsm_handle) {
  CANConfig *can = fsm->context;
  FSM_ADD_TRANSITION(can->rx_event, can_rx_fsm_handle);
}

static StatusCode prv_handle_data_msg(CANConfig *can, const CANMessage *rx_msg) {
  CANRxHandler *handler = can_rx_get_handler(&can->rx_handlers, rx_msg->msg_id);
  CANAckStatus ack_status = CAN_ACK_STATUS_OK;
  StatusCode ret = STATUS_CODE_OK;

  if (handler != NULL) {
    ret = handler->callback(&rx_msg, handler->context, &ack_status);
    status_ok_or_return(ret);
  }

  if (CAN_ACK_EXPECTED(rx_msg)) {
    CANMessage ack = {
      .msg_id = rx_msg->msg_id,
      .type = CAN_MSG_TYPE_ACK,
      .dlc = sizeof(ack_status),
      .data = ack_status
    };

    ret = can_transmit(can, &ack, NULL);
    status_ok_or_return(ret);
  }

  return ret;
}

static void prv_handle_rx(FSM *fsm, const Event *e, void *context) {
  CANConfig *can = context;
  CANMessage rx_msg = { 0 };

  StatusCode result = can_queue_pop(&can->rx_queue, &rx_msg);
  if (result != STATUS_CODE_OK) {
    // We had a mismatch between number of events and number of messages, so return silently
    // Alternatively, we could use the data value of the event.
    return;
  }

  switch(rx_msg.type) {
    case CAN_MSG_TYPE_ACK:
      result = can_ack_handle_msg(&can->ack_requests, &rx_msg);

      break;
    case CAN_MSG_TYPE_DATA:
      result = prv_handle_data_msg(can, &rx_msg);

      break;
    default:
      status_msg(STATUS_CODE_UNREACHABLE, "CAN RX: Invalid type");

      return;
      // error
      break;
  }

  if (result != STATUS_CODE_OK) {
    // Error? TODO: figure out what we should do with FSM errors
    return;
  }

  if (can_queue_size(&can->rx_queue) > 0) {
    // Re-raise event to process the next element in the queue
    event_raise(e->id, e->data);
  }
}

StatusCode can_rx_fsm_init(FSM *rx_fsm, CANConfig *can) {
  if (rx_fsm == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  fsm_init(rx_fsm, "can_rx_fsm", &can_rx_fsm_handle, can);
  fsm_state_init(can_rx_fsm_handle, prv_handle_rx);
}
