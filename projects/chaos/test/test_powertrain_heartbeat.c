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
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define NUM_CAN_RX_HANDLERS 4

static CANStorage s_storage;
static CANRxHandler s_rx_handlers[NUM_CAN_RX_HANDLERS];

static void prv_transmit(uint16_t times) {
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  for (uint16_t i = 0; i < times; i++) {
    // TX
    do {
      status = event_process(&e);
    } while (status == STATUS_CODE_EMPTY);
    TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_TX, e.id);
    TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    // RX
    do {
      status = event_process(&e);
    } while (status == STATUS_CODE_EMPTY);
    TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_RX, e.id);
    TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    // TX
    do {
      status = event_process(&e);
    } while (status == STATUS_CODE_EMPTY);
    TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_TX, e.id);
    TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    // RX
    do {
      status = event_process(&e);
    } while (status == STATUS_CODE_EMPTY);
    TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_RX, e.id);
    TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  }
}

void setup_test(void) {
  interrupt_init();
  event_queue_init();
  gpio_init();
  soft_timer_init();

  CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_125KBPS,
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
  prv_transmit(3);

  // Watchdog should activate.
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
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

  prv_transmit(1);

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

  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_TX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  msg.source_id = SYSTEM_CAN_DEVICE_PLUTUS;
  TEST_ASSERT_OK(can_ack_handle_msg(&s_storage.ack_requests, &msg));
  msg.source_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS;
  TEST_ASSERT_OK(can_ack_handle_msg(&s_storage.ack_requests, &msg));
  msg.source_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER;
  TEST_ASSERT_OK(can_ack_handle_msg(&s_storage.ack_requests, &msg));

  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_RX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_TX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_RX, e.id);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  // Skip the two messages that should send
  prv_transmit(2);

  // Watchdog should trigger.
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY, e.id);
}
