#include <stdbool.h>
#include <stdlib.h>

#include "can.h"
#include "can_msg_defs.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "wait.h"

#define RX 1
#define TX 2
#define FAULT 3
#define NUM_RX_HANDLERS 1

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[NUM_RX_HANDLERS];

static StatusCode prv_handle_can_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  LOG_DEBUG("ID: %d\n", msg->msg_id);
  return STATUS_CODE_OK;
}

int main(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = RX,
    .tx_event = TX,
    .fault_event = FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
  can_init(&s_can_storage, &can_settings, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
  can_register_rx_default_handler(prv_handle_can_rx, NULL);
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  while (true) {
    // Tight event loop
    do {
      status = event_process(&e);
      // TODO(ELEC-105): Validate nothing gets stuck here.
      if (status == STATUS_CODE_EMPTY) {
        wait();
      }
    } while (status != STATUS_CODE_OK);

    // Event Processing:

    // TODO(ELEC-105): At least one of the following should respond with either a boolean true or
    // a STATUS_CODE_OK for each emitted message. Consider adding a requirement that this is the
    // case with a failure resulting in faulting into Emergency.
    fsm_process_event(CAN_FSM, &e);
  }

  // Not reached.
  return EXIT_SUCCESS;
}
