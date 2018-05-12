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
#include "exported_enums.h"
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

static EEChargerSetRelayState s_response;
static GenericCanNetwork s_generic_can;
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[TEST_NOTIFY_NUM_CAN_RX_HANDLERS];

// GenericCanRxCb
static void prv_callback(const GenericCanMsg *msg, void *context) {
  (void)msg;
  EEChargerSetRelayState *state = context;
  if (*state < NUM_EE_CHARGER_SET_RELAY_STATES) {
    CAN_TRANSMIT_CHARGER_SET_RELAY_STATE(*state);
  }
}

static void prv_check_state_from_fsm(uint16_t id) {
  Event e;
  StatusCode status;
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_EQUAL(id, e.id);
}

static void prv_transmit_half() {
  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;
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

static void prv_transmit_full(uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    prv_transmit_half();
    prv_transmit_half();
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
  TEST_ASSERT_OK(
      generic_can_register_rx((GenericCan *)&s_generic_can, prv_callback, GENERIC_CAN_EMPTY_MASK,
                              SYSTEM_CAN_MESSAGE_CHARGER_CONN_STATE, false, &s_response));
  can_interval_init();
}

void teardown_test(void) {}

void test_notify(void) {
  TEST_ASSERT_OK(notify_init((GenericCan *)&s_generic_can, TEST_NOTIFY_PERIOD_S,
                             TEST_NOTIFY_WATCHDOG_PERIOD_S));

  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;

  // Charge
  s_response = EE_CHARGER_SET_RELAY_STATE_CLOSE;
  notify_post();
  // Do twice to ensure the watchdog doesn't get triggered
  prv_transmit_full(1);
  prv_check_state_from_fsm(CHARGER_EVENT_START_CHARGING);

  prv_transmit_full(1);
  prv_check_state_from_fsm(CHARGER_EVENT_START_CHARGING);

  // Don't charge
  s_response = EE_CHARGER_SET_RELAY_STATE_OPEN;
  prv_transmit_full(1);
  prv_check_state_from_fsm(CHARGER_EVENT_STOP_CHARGING);

  // Watchdog
  s_response = NUM_EE_CHARGER_SET_RELAY_STATES;
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
  s_response = EE_CHARGER_SET_RELAY_STATE_CLOSE;
  prv_transmit_full(1);
  prv_check_state_from_fsm(CHARGER_EVENT_START_CHARGING);

  // Send a singular disconnect message.
  s_response = NUM_EE_CHARGER_SET_RELAY_STATES;  // Don't respond.
  notify_cease();
  prv_transmit_half();

  delay_ms(1100);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
