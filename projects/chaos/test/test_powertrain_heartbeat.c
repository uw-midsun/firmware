#include "powertrain_heartbeat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_hw.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define NUM_CAN_RX_HANDLERS 4

// Skip the sequencer protection of the event queue since it would overly complicate the test.
bool TEST_MOCK(sequencer_fsm_event_raise)(ChaosEventSequence sequence) {
  event_raise(sequence, 0);
  return true;
}

static CANStorage s_storage;
static CANRxHandler s_rx_handlers[NUM_CAN_RX_HANDLERS];
static Event s_tx_event = { CHAOS_EVENT_CAN_TX, 0 };
static Event s_rx_event = { CHAOS_EVENT_CAN_RX, 0 };

void setup_test(void) {
  interrupt_init();
  event_queue_init();
  gpio_init();
  soft_timer_init();

  CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&settings, &s_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
  can_add_filter(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT);
  TEST_ASSERT_OK(powertrain_heartbeat_init());
}

void teardown_test(void) {}

void test_powertrain_heartbeat_watchdog(void) {
  Event e = { 0 };
  volatile StatusCode status = NUM_STATUS_CODES;
  e.id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE;
  TEST_ASSERT_TRUE(powertrain_heartbeat_process_event(&e));

  // Send 3 times (all three will have ack failures).
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);

  // Watchdog should activate.
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY, e.id);

  // No more should activate.
  delay_ms(POWERTRAIN_HEARTBEAT_WATCHDOG_MS);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_stop_heartbeat(void) {
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  e.id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE;
  TEST_ASSERT_TRUE(powertrain_heartbeat_process_event(&e));

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);

  e.id = CHAOS_EVENT_SEQUENCE_EMERGENCY;
  TEST_ASSERT_TRUE(powertrain_heartbeat_process_event(&e));

  delay_ms(POWERTRAIN_HEARTBEAT_WATCHDOG_MS + 100);
  // No activation should occur.
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_kick_watchdog(void) {
  CANMessage msg = {
    .type = CAN_MSG_TYPE_ACK,
    .msg_id = SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
  };

  Event e = { 0 };
  volatile StatusCode status = NUM_STATUS_CODES;
  e.id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE;
  TEST_ASSERT_TRUE(powertrain_heartbeat_process_event(&e));

  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_TX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  msg.source_id = SYSTEM_CAN_DEVICE_PLUTUS;
  TEST_ASSERT_OK(can_ack_handle_msg(&s_storage.ack_requests, &msg));
  msg.source_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS;
  TEST_ASSERT_OK(can_ack_handle_msg(&s_storage.ack_requests, &msg));
  msg.source_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER;
  TEST_ASSERT_OK(can_ack_handle_msg(&s_storage.ack_requests, &msg));

  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_RX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  MS_TEST_HELPER_CAN_TX_RX(s_tx_event, s_rx_event);

  // Skip the two messages that should send
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);

  // Watchdog should trigger.
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY, e.id);
}
