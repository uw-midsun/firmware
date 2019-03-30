#include "can.h"
#include "can_transmit.h"
#include "cc_cfg.h"
#include "cc_input_event.h"
#include "delay.h"
#include "drive_output.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "steering_indicator.h"
#include "test_helpers.h"
#include "unity.h"

CanStorage s_can_storage;

void setup_test(void) {
  interrupt_init();
  event_queue_init();
  soft_timer_init();

  const CanSettings can_settings = {
    .device_id = CC_CFG_CAN_DEVICE_ID,
    .bitrate = CC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CENTER_CONSOLE_CAN_RX,
    .tx_event = INPUT_EVENT_CENTER_CONSOLE_CAN_TX,
    .fault_event = INPUT_EVENT_CENTER_CONSOLE_CAN_FAULT,
    .tx = CC_CFG_CAN_RX,
    .rx = CC_CFG_CAN_TX,
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  can_add_filter(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT);

  TEST_ASSERT_OK(drive_output_init(drive_output_global(), 0, 0));
  drive_output_set_enabled(drive_output_global(), true);
  TEST_ASSERT_OK(steering_indicator_init());
}

void teardown_test(void) {}

void test_steering_indicator_cruise(void) {
  const uint16_t cruise = 123;
  CAN_TRANSMIT_STEERING_OUTPUT(cruise);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // No event should be raised by the steering indicator
  Event e;
  TEST_ASSERT_NOT_OK(event_process(&e));

  DriveOutputStorage* storage = drive_output_global();
  TEST_ASSERT_EQUAL(cruise, storage->data[DRIVE_OUTPUT_SOURCE_CRUISE]);
}
