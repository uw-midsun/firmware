#include "can_fsm.h"
#include "can_rx.h"
#include "can_hw.h"
#include "can.h"

FSM_DECLARE_STATE(can_rx_fsm_handle);
FSM_DECLARE_STATE(can_tx_fsm_handle);

FSM_STATE_TRANSITION(can_rx_fsm_handle) {
  CANStorage *can_storage = fsm->context;
  FSM_ADD_TRANSITION(can_storage->rx_event, can_rx_fsm_handle);
  FSM_ADD_TRANSITION(can_storage->tx_event, can_tx_fsm_handle);
}

FSM_STATE_TRANSITION(can_tx_fsm_handle) {
  CANStorage *can_storage = fsm->context;
  FSM_ADD_TRANSITION(can_storage->rx_event, can_rx_fsm_handle);
  FSM_ADD_TRANSITION(can_storage->tx_event, can_tx_fsm_handle);
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
  StatusCode result = can_fifo_pop(&can_storage->rx_fifo, &rx_msg);
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

// TODO: flesh out design
// now assuming that TX is always 1-to-1
// basically want to be able to rate-limit tx attempts - delay processing until TX happens
static void prv_handle_tx(FSM *fsm, const Event *e, void *context) {
  CANStorage *can_storage = context;
  CANMessage tx_msg = { 0 };

  // printf("E: TX (%d) - %d queued\n", e->data, can_fifo_size(&can_storage->tx_fifo));

  StatusCode result = can_fifo_peek(&can_storage->tx_fifo, &tx_msg);
  if (result != STATUS_CODE_OK) {
    // Mismatch
    return;
  }

  CANId msg_id = {
    .source_id = can_storage->device_id,
    .type = tx_msg.type,
    .msg_id = tx_msg.msg_id
  };

  // TODO: Semes to be some sort of deadlock if the bus is flooded - why does this not turn off the
  // bus?
  // If added to mailbox, pop message from the TX queue
  StatusCode ret = can_hw_transmit(msg_id.raw, tx_msg.data_u8, tx_msg.dlc);
  if (ret == STATUS_CODE_OK) {
    can_fifo_pop(&can_storage->tx_fifo, NULL);
    // printf("pop %d\n", tx_msg.data_u32[0]);
  } else if (can_hw_bus_status() != CAN_HW_BUS_STATUS_OK) {
    // printf("bus error??? %d\n", can_hw_bus_status());
  } else {
    // CAN TX attempt failed - record attempt for TX ready interrupt
    // Once TX is ready again, it will raise new TX events
    // technically, there's a race condition here - if the TX ready interrupt fires before we
    // increment, then we'll fail to kickstart the TX loop so the failed TX won't occur until the
    // next transmit.
    can_storage->num_failed_tx++;
    // printf("%d TX fail (%d) %d\n", tx_msg.data_u32[0], can_fifo_size(&can_storage->tx_fifo),
    //        can_storage->num_failed_tx);
  }
}

StatusCode can_fsm_init(FSM *fsm, CANStorage *can_storage) {
  if (fsm == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  fsm_init(fsm, "can_fsm", &can_rx_fsm_handle, can_storage);
  fsm_state_init(can_rx_fsm_handle, prv_handle_rx);
  fsm_state_init(can_tx_fsm_handle, prv_handle_tx);

  return STATUS_CODE_OK;
}
