#include "charger.h"

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CHARGER_NUM_CAN_RX_HANDLERS 1

static CANStorage s_storage;
static CANRxHandler s_rx_handlers[TEST_CHARGER_NUM_CAN_RX_HANDLERS];
static EEChargerSetRelayState s_expected_state = NUM_EE_CHARGER_SET_RELAY_STATES;

static void prv_charger_can_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  EEChargerSetRelayState state;
  CAN_UNPACK_CHARGER_SET_RELAY_STATE(msg, &state);
  TEST_ASSERT_EQUAL(s_expected_state, state);
}

static void prv_transmit() {}

void setup_test(void) {
  interrupt_init();
  gpio_init();
  event_queue_init();

  const CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .loopback = true,
  };

  can_init(&settings, &s_storage, s_rx_handlers, TEST_CHARGER_NUM_CAN_RX_HANDLERS);
  TEST_ASSERT_OK(charger_init());
}

void teardown_test(void) {}

void test_charger_state(void) {
  StatusCode status = NUM_STATUS_CODES;
  Event e;

  // Check no send happens if the charger hasn't been connected.
  TEST_ASSERT_OK(charger_set_state(EE_CHARGER_SET_RELAY_STATE_CLOSE));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
  TEST_ASSERT_OK(charger_set_state(EE_CHARGER_SET_RELAY_STATE_OPEN));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Connect Charger.
  s_expected_state = EE_CHARGER_SET_RELAY_STATE_OPEN;
  // TODO(ELEC-355): Convert to notification message when codegen-tooling is updated.
  CAN_TRANSMIT_CHARGER_CONN_STATE(EE_CHARGER_CONN_STATE_CONNECTED);
  // TX and RX for notification and command.
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  s_expected_state = EE_CHARGER_SET_RELAY_STATE_CLOSE;
  TEST_ASSERT_OK(charger_set_state(s_expected_state));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
  // TX and RX for the command.
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  // Disconnect Charger.
  // TODO(ELEC-355): Convert to notification message when codegen-tooling is updated.
  CAN_TRANSMIT_CHARGER_CONN_STATE(EE_CHARGER_CONN_STATE_DISCONNECTED);
  // TX and RX for notification.
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  // Check no more sends occur.
  TEST_ASSERT_OK(charger_set_state(EE_CHARGER_SET_RELAY_STATE_OPEN));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
