#include "can.h"
#include "can_transmit.h"
#include "cc_cfg.h"
#include "cc_input_event.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pedal_indicator.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();

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
  can_add_filter(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT);

  TEST_ASSERT_OK(pedal_indicator_init());
}

void teardown_test(void) {}

void test_pedal_indicator_brake_pressed(void) {
  uint16_t mech_brake = EE_PEDAL_OUTPUT_MECH_THRESHOLD + 1;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, 0, mech_brake);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CENTER_CONSOLE_MECH_BRAKE_PRESSED, e.id);
}

void test_pedal_indicator_brake_released(void) {
  uint16_t mech_brake = EE_PEDAL_OUTPUT_MECH_THRESHOLD - 1;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, 0, mech_brake);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CENTER_CONSOLE_MECH_BRAKE_RELEASED, e.id);
}
