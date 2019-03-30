#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "direction_indicator.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pc_cfg.h"
#include "pc_input_event.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = PC_CFG_CAN_DEVICE_ID,
    .bitrate = PC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_PEDAL_CAN_RX,
    .tx_event = INPUT_EVENT_PEDAL_CAN_TX,
    .fault_event = INPUT_EVENT_PEDAL_CAN_FAULT,
    .tx = PC_CFG_CAN_RX,
    .rx = PC_CFG_CAN_TX,
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  can_add_filter(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT);

  TEST_ASSERT_OK(direction_indicator_init());
}

void teardown_test(void) {}

void test_direction_indicator_neutral(void) {
  uint16_t direction = EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL;
  CAN_TRANSMIT_DRIVE_OUTPUT(0, direction, 0, 0);

  Event e;
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_PEDAL_CAN_TX, INPUT_EVENT_PEDAL_CAN_RX);

  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_DIRECTION_STATE_NEUTRAL, e.id);
}

void test_direction_indicator_forward(void) {
  uint16_t direction = EE_DRIVE_OUTPUT_DIRECTION_FORWARD;
  CAN_TRANSMIT_DRIVE_OUTPUT(0, direction, 0, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_PEDAL_CAN_TX, INPUT_EVENT_PEDAL_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_DIRECTION_STATE_FORWARD, e.id);
}

void test_direction_indicator_reverse(void) {
  uint16_t direction = EE_DRIVE_OUTPUT_DIRECTION_REVERSE;
  CAN_TRANSMIT_DRIVE_OUTPUT(0, direction, 0, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_PEDAL_CAN_TX, INPUT_EVENT_PEDAL_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_DIRECTION_STATE_REVERSE, e.id);
}
