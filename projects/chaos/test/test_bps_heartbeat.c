#include "bps_heartbeat.h"

#include <stddef.h>
#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static CANStorage s_storage;

static StatusCode prv_bps_ack_request(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                      uint16_t remaining, void *context) {
  (void)context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_DEVICE_CHAOS, device);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, status);
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, msg_id);
  TEST_ASSERT_EQUAL(0, remaining);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
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

  can_init(&s_storage, &settings);
  TEST_ASSERT_OK(bps_heartbeat_init());
}

void teardown_test(void) {}

void test_bps_heartbeat_watchdog_kick(void) {
  const CANAckRequest ack_req = {
    .callback = prv_bps_ack_request,
    .context = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS),
  };

  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;

  // Auto Start
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_req, EE_BPS_HEARTBEAT_STATE_OK);
  // Send HB
  // TX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // In theory these latter three events are in indeterminate order but it doesn't matter
  // RX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // HB Timer is started

  // Ack HB
  // TX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));
  // RX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  delay_ms(BPS_HEARTBEAT_EXPECTED_PERIOD_MS / 2);

  // Send the HB again
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_req, EE_BPS_HEARTBEAT_STATE_OK);
  // Send HB
  // TX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // In theory these latter three events are in indeterminate order but it doesn't matter
  // RX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // HB Timer is started

  // Ack HB
  // TX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));
  // RX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // Delay long enough the first watchdog would expire but not the second.
  delay_ms(BPS_HEARTBEAT_EXPECTED_PERIOD_MS * 3 / 4);
  // Verify no event is raised
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Verify that the watchdog actually expires.
  delay_ms(BPS_HEARTBEAT_EXPECTED_PERIOD_MS);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY, e.id);

  // Verify that it auto restarts.
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_req, EE_BPS_HEARTBEAT_STATE_OK);
  // Send HB
  // TX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // In theory these latter three events are in indeterminate order but it doesn't matter
  // RX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // HB Timer is started

  // Ack HB
  // TX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));
  // RX
  do {
    status = event_process(&e);
  } while (status == STATUS_CODE_EMPTY);
  TEST_ASSERT_TRUE(can_process_event(&e));

  // Verify that the watchdog actually expires.
  delay_ms(BPS_HEARTBEAT_EXPECTED_PERIOD_MS + BPS_HEARTBEAT_EXPECTED_PERIOD_MS / 10);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY, e.id);
}

void test_bps_heartbeat_watchdog_timeout(void) {
  TEST_ASSERT_OK(bps_heartbeat_start());
  delay_ms(BPS_HEARTBEAT_EXPECTED_PERIOD_MS + BPS_HEARTBEAT_EXPECTED_PERIOD_MS / 10);

  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY, e.id);

  // Verify the watchdog is stopped after receiving the first time.
  delay_ms(BPS_HEARTBEAT_EXPECTED_PERIOD_MS + BPS_HEARTBEAT_EXPECTED_PERIOD_MS / 10);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
