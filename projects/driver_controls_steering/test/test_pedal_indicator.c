#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pedal_indicator.h"
#include "sc_cfg.h"
#include "sc_input_event.h"
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
    .device_id = SC_CFG_CAN_DEVICE_ID,
    .bitrate = SC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_STEERING_CAN_RX,
    .tx_event = INPUT_EVENT_STEERING_CAN_TX,
    .fault_event = INPUT_EVENT_STEERING_CAN_FAULT,
    .tx = SC_CFG_CAN_RX,
    .rx = SC_CFG_CAN_TX,
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

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_MECH_BRAKE_PRESSED, e.id);
}

void test_pedal_indicator_brake_released(void) {
  uint16_t mech_brake = EE_PEDAL_OUTPUT_MECH_THRESHOLD - 1;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, 0, mech_brake);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_MECH_BRAKE_RELEASED, e.id);
}

void test_pedal_indicator_throttle_brake(void) {
  uint16_t throttle_state = EE_THROTTLE_BRAKE;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, throttle_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_MECH_BRAKE_RELEASED, e.id);

  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_PEDAL_BRAKE, e.id);
}

void test_pedal_indicator_throttle_coast(void) {
  uint16_t throttle_state = EE_THROTTLE_COAST;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, throttle_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_MECH_BRAKE_RELEASED, e.id);

  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_PEDAL_COAST, e.id);
}

void test_pedal_indicator_throttle_accel(void) {
  uint16_t throttle_state = EE_THROTTLE_ACCEL;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, throttle_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_MECH_BRAKE_RELEASED, e.id);

  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_PEDAL_ACCEL, e.id);
}

void test_pedal_inidicator_throttle_fault(void) {
  uint16_t throttle_state = EE_THROTTLE_FAULT;
  CAN_TRANSMIT_PEDAL_OUTPUT(0, throttle_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);
  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_PEDAL_FAULT, e.id);
}
