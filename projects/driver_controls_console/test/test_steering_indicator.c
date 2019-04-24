#include "can.h"
#include "can_transmit.h"
#include "cc_cfg.h"
#include "cc_input_event.h"
#include "delay.h"
#include "drive_output.h"
#include "event_queue.h"
#include "exported_enums.h"
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

void test_steering_indicator_analog_distance(void) {
  const uint16_t control_stalk_analog_state = EE_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL;

  CAN_TRANSMIT_STEERING_OUTPUT(control_stalk_analog_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL, e.id);
}

void test_steering_indicator_analog_speed(void) {
  const uint16_t control_stalk_analog_state = EE_CONTROL_STALK_ANALOG_CC_SPEED_MINUS;

  CAN_TRANSMIT_STEERING_OUTPUT(control_stalk_analog_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS, e.id);
}

void test_steering_indicator_analog_resume(void) {
  const uint16_t control_stalk_analog_state = EE_CONTROL_STALK_ANALOG_CC_RESUME;

  CAN_TRANSMIT_STEERING_OUTPUT(control_stalk_analog_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, e.id);
}

void test_steering_indicator_analog_turn_signal(void) {
  const uint16_t control_stalk_analog_state = EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_RIGHT;

  CAN_TRANSMIT_STEERING_OUTPUT(control_stalk_analog_state, 0);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT, e.id);
}

void test_steering_indicator_digital_cc_set(void) {
  const uint16_t control_stalk_digital_state = EE_CONTROL_STALK_DIGITAL_CC_SET_RELEASED;

  CAN_TRANSMIT_STEERING_OUTPUT(0, control_stalk_digital_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // process analog event first
  Event a;
  event_process(&a);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED, e.id);
}

void test_steering_indicator_digital_cc_lane_assist(void) {
  const uint16_t control_stalk_digital_state = EE_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED;

  CAN_TRANSMIT_STEERING_OUTPUT(0, control_stalk_digital_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // process analog event first
  Event a;
  event_process(&a);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED, e.id);
}

void test_steering_indicator_digital_high_beam_fwd(void) {
  const uint16_t control_stalk_digital_state = EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_FWD_RELEASED;

  CAN_TRANSMIT_STEERING_OUTPUT(0, control_stalk_digital_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // process analog event first
  Event a;
  event_process(&a);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, e.id);
}

void test_steering_indicator_digital_high_beam_back(void) {
  const uint16_t control_stalk_digital_state = EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_BACK_PRESSED;

  CAN_TRANSMIT_STEERING_OUTPUT(0, control_stalk_digital_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // process analog event first
  Event a;
  event_process(&a);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_PRESSED, e.id);
}
