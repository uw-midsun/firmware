#include "notify.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_interval.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "charger_events.h"
#include "delay.h"
#include "event_queue.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "generic_can_network.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_NOTIFY_PERIOD_S 1
#define TEST_NOTIFY_WATCHDOG_PERIOD_S 2
#define TEST_NOTIFY_NUM_CAN_RX_HANDLERS 4

static uint8_t s_response;
static GenericCanNetwork s_generic_can;
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[TEST_NOTIFY_NUM_CAN_RX_HANDLERS];

// GenericCanRxCb
static void prv_callback(const GenericCanMsg *msg, void *context) {
  (void)msg;
  uint8_t *state = context;
  if (*state < 2) {
    // TODO(ELEC-355): Convert to set state message when codegen-tooling is updated.
    CAN_TRANSMIT_CHARGING_PERMISSION(*state);
  }
}

static void prv_transmit(uint8_t times) {
  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;
  for (uint8_t i = 0; i < times; i++) {
    // TX
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
    // TX
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
  }
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  const CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHARGER,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .rx_event = CHARGER_EVENT_CAN_RX,
    .tx_event = CHARGER_EVENT_CAN_TX,
    .fault_event = CHARGER_EVENT_CAN_FAULT,
    .loopback = true,
  };

  TEST_ASSERT_OK(
      can_init(&can_settings, &s_can_storage, s_rx_handlers, TEST_NOTIFY_NUM_CAN_RX_HANDLERS));
  TEST_ASSERT_OK(generic_can_network_init(&s_generic_can));
  TEST_ASSERT_OK(generic_can_register_rx((GenericCan *)&s_generic_can, prv_callback,
                                         GENERIC_CAN_EMPTY_MASK, SYSTEM_CAN_MESSAGE_CHARGING_REQ,
                                         false, &s_response));
  can_interval_init();
}

void teardown_test(void) {}

void test_permissions(void) {
  TEST_ASSERT_OK(notify_init((GenericCan *)&s_generic_can, TEST_NOTIFY_PERIOD_S,
                             TEST_NOTIFY_WATCHDOG_PERIOD_S));

  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;

  // Charge
  s_response = true;
  notify_post();
  // Do twice to ensure the watchdog doesn't get triggered
  prv_transmit(1);
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_START_CHARGING, e.id);

  prv_transmit(1);
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_START_CHARGING, e.id);

  // Don't charge
  s_response = false;
  prv_transmit(1);
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_STOP_CHARGING, e.id);

  // Watchdog
  s_response = UINT8_MAX;
  e.id = UINT16_MAX;
  while (e.id != CHARGER_EVENT_STOP_CHARGING) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHARGER_EVENT_CAN_RX || e.id == CHARGER_EVENT_CAN_TX) {
      TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    }
  }

  // Check still running
  // Charge
  s_response = true;
  prv_transmit(1);
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(CHARGER_EVENT_START_CHARGING, e.id);

  // Send a singular disconnect message.
  s_response = 3;
  notify_cease();
  // TX
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

  delay_ms(1100);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
