// Basically, this module glues all the components of CAN together.
//
// It hooks into the CAN HW callbacks:
// - TX ready: When a transmit is requested, it will first try to immediately send the message.
//             If that fails, then the message is pushed into a queue. When the TX ready callback
//             runs, it tries to send any messages in that queue. This callback should run before
//             code that tries to send messages can run, so we should be okay. Just in case our
//             CAN queue isn't actually interrupt-safe, be warned that a race condition could occur.
// - Message RX: When the message RX callback runs, we just push the message into a queue and
//               raise an event. When that event is processed in the main loop
//               (can_fsm_process_event), we find the associated callback and run it.
//               In the case of an ACK, we update the associated pending ACK request.
// - Bus error: In case of a bus error, we set a timer and wait to see if the bus has recovered
//              after the timeout. If it's still down, we raise an event.
#include "can.h"
#include <string.h>
#include "soft_timer.h"
#include "can_hw.h"
#include "can_rx_fsm.h"
#include "log.h"

#define CAN_BUS_OFF_RECOVERY_TIME_MS 500

// Attempts to transmit the specified message using the HW TX, overwriting the source device.
StatusCode prv_transmit(const CANMessage *msg);

// Handler for CAN HW TX ready events
// Attempts to transmit any messages in the TX queue
void prv_tx_handler(void *context);

// Handler for CAN HW messaged RX events
// Dumps received messages to the RX queue and raises an event for the messages to be processed.
void prv_rx_handler(void *context);

// Bus error timer callback
// Checks if the bus has recovered, raising the fault event if still off
void prv_bus_error_timeout_handler(SoftTimerID timer_id, void *context);

// Handler for CAN HW bus error events
// Starts a timer to check for bus recovery
void prv_bus_error_handler(void *context);

static CANStorage *s_can_storage;

StatusCode can_init(const CANSettings *settings, CANStorage *storage,
                    CANRxHandler *rx_handlers, size_t num_rx_handlers) {
  if (settings->device_id >= CAN_MSG_MAX_DEVICES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid device ID");
  }

  memset(storage, 0, sizeof(*storage));
  storage->rx_event = settings->rx_event;
  storage->fault_event = settings->fault_event;
  storage->device_id = settings->device_id;

  s_can_storage = storage;
  LOG_DEBUG("Setting s_can_storage to %p\n", s_can_storage);

  CANHwSettings can_hw_settings = {
    .bus_speed = settings->bus_speed,
    .loopback = settings->loopback,
    .tx = settings->tx,
    .rx = settings->rx
  };
  can_hw_init(&can_hw_settings);

  can_rx_fsm_init(&s_can_storage->fsm, s_can_storage);
  can_queue_init(&s_can_storage->tx_queue);
  can_queue_init(&s_can_storage->rx_queue);
  can_ack_init(&s_can_storage->ack_requests);
  can_rx_init(&s_can_storage->rx_handlers, rx_handlers, num_rx_handlers);

  can_hw_register_callback(CAN_HW_EVENT_TX_READY, prv_tx_handler, s_can_storage);
  can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_rx_handler, s_can_storage);
  can_hw_register_callback(CAN_HW_EVENT_BUS_ERROR, prv_bus_error_handler, s_can_storage);

  return STATUS_CODE_OK;
}

StatusCode can_add_filter(CANMessageID msg_id) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  CANId can_id = {
    .msg_id = msg_id
  }, mask = {
    .msg_id = UINT16_MAX
  };

  return can_hw_add_filter(can_id.raw, mask.raw);
}

StatusCode can_register_rx_default_handler(CANRxHandlerCb handler, void *context) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  return can_rx_register_default_handler(&s_can_storage->rx_handlers, handler, context);
}

StatusCode can_register_rx_handler(CANMessageID msg_id, CANRxHandlerCb handler, void *context) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  return can_rx_register_handler(&s_can_storage->rx_handlers, msg_id, handler, context);
}

StatusCode can_transmit(const CANMessage *msg, const CANAckRequest *ack_request) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg->msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  if (ack_request != NULL) {
    if (!CAN_MSG_IS_CRITICAL(msg)) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: ACK requested for non-critical message");
    }

    StatusCode ret = can_ack_add_request(&s_can_storage->ack_requests, msg->msg_id, ack_request);
    status_ok_or_return(ret);
  }

  // We could push the message first and attempt to transmit the message with the lowest ID,
  // but there really isn't any point. We get an interrupt whenever a TX mailbox is empty and
  // attempt to fill it, so that should usually take precedence over this function.
  StatusCode tx_result = prv_transmit(msg);
  // TODO: Figure out why this is causing duplicates
  // if (tx_result != STATUS_CODE_OK) {
  //   // As long as the timer for the ACK request is started, it's okay for the message to fail.
  //   // An optimization would be to fail early and signal that as an error.
  //   return can_queue_push(&s_can_storage->tx_queue, msg);
  // }

  return STATUS_CODE_OK;
}

FSM *can_get_fsm(void) {
  if (s_can_storage == NULL) {
    return NULL;
  }

  return &s_can_storage->fsm;
}

StatusCode prv_transmit(const CANMessage *msg) {
  CANId msg_id = {
    .source_id = s_can_storage->device_id,
    .type = msg->type,
    .msg_id = msg->msg_id
  };

  return can_hw_transmit(msg_id.raw, msg->data_u8, msg->dlc);
}

void prv_tx_handler(void *context) {
  CANStorage *can_storage = context;
  CANMessage tx_msg;

  // TODO: add rate limiting for retries
  while (can_queue_size(&can_storage->tx_queue) > 0) {
    can_queue_pop(&can_storage->tx_queue, &tx_msg);

    // printf("Actually sending msg %d (0x%x%x)\n", tx_msg.msg_id, tx_msg.data_u32[1], tx_msg.data_u32[0]);

    StatusCode result = prv_transmit(&tx_msg);

    if (result != STATUS_CODE_OK) {
      return;
    }

    // can_queue_pop(&can_storage->tx_queue, NULL);
  }
}

// TODO: Flesh this out
// Now assumes that the ISR will be fired once for each message and we should only process one at a
// time
// TODO: Why are we receiving multiple times?
void prv_rx_handler(void *context) {
  CANStorage *can_storage = context;
  CANId rx_id = { 0 };
  CANMessage rx_msg = { 0 };
  size_t counter = 0;

  can_hw_receive(&rx_id.raw, &rx_msg.data, &rx_msg.dlc);
  CAN_MSG_SET_RAW_ID(&rx_msg, rx_id.raw);
  // printf("rx msg %d\n", rx_msg.msg_id);

  StatusCode result = can_queue_push(&can_storage->rx_queue, &rx_msg);
  // TODO: add error handling for FSMs
  if (result != STATUS_CODE_OK) {
    // printf("error pushing msg to rx queue\n");
    return;
  }

  // printf("raising rx event\n");
  event_raise(can_storage->rx_event, 1);
}

void prv_bus_error_timeout_handler(SoftTimerID timer_id, void *context) {
  CANStorage *can_storage = context;

  printf("bus error timeout\n");

  CANHwBusStatus status = can_hw_bus_status();

  if (status == CAN_HW_BUS_STATUS_OFF) {
    printf("raising fault event\n");
    event_raise(can_storage->fault_event, 0);
  }
}

void prv_bus_error_handler(void *context) {
  CANStorage *can_storage = context;

  printf("Bus error\n");

  soft_timer_start_millis(CAN_BUS_OFF_RECOVERY_TIME_MS, prv_bus_error_timeout_handler,
                          can_storage, NULL);
}
