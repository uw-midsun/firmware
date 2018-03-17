#include "generic_can_network.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "charger_events.h"
#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "generic_can.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static GenericCanNetwork s_can;
static CANStorage s_storage;
static CANRxHandler s_rx_handlers[NUM_GENERIC_CAN_RX_HANDLERS];

// GenericCanRxCb
static void prv_can_rx_callback(const GenericCanMsg *msg, void *context) {
  (void)msg;
  uint8_t *data = context;
  *data += 1;
  LOG_DEBUG("Callback\n");
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();

  const CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHARGER,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .rx_event = CHARGER_EVENT_CAN_RX,
    .tx_event = CHARGER_EVENT_CAN_TX,
    .fault_event = CHARGER_EVENT_CAN_FAULT,
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&can_settings, &s_storage, s_rx_handlers, NUM_GENERIC_CAN_RX_HANDLERS));

  TEST_ASSERT_OK(generic_can_network_init(&s_can));
}

void teardown_test(void) {}

void test_generic_can(void) {
  GenericCan *can = (GenericCan *)&s_can;

  volatile uint8_t counter = 0;

  const CANId raw_id = {
    .source_id = SYSTEM_CAN_DEVICE_CHARGER,
    .msg_id = 30,
    .type = CAN_MSG_TYPE_DATA,
  };

  const GenericCanMsg msg = {
    .id = raw_id.raw,
    .data = 255,
    .dlc = 1,
    .extended = false,
  };

  TEST_ASSERT_OK(generic_can_register_rx(can, prv_can_rx_callback, raw_id.msg_id, &counter));

  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;
  // TX
  TEST_ASSERT_OK(generic_can_tx(can, &msg));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_CAN_TX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // RX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_CAN_RX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // Callback is triggered.
  TEST_ASSERT_EQUAL(1, counter);

  // Queue Empty
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Mask the rx handler
  TEST_ASSERT_OK(generic_can_disable_rx(can, raw_id.msg_id));

  // TX
  TEST_ASSERT_OK(generic_can_tx(can, &msg));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_CAN_TX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // No RX
  TEST_ASSERT_EQUAL(1, counter);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Unmask the rx handler
  TEST_ASSERT_OK(generic_can_enable_rx(can, raw_id.msg_id));
  // TX
  TEST_ASSERT_OK(generic_can_tx(can, &msg));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_CAN_TX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // RX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_CAN_RX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // Callback is triggered.
  TEST_ASSERT_EQUAL(2, counter);

  // Queue empty
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
