#include "can.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "sc_cfg.h"
#include "sc_input_event.h"
#include "soft_timer.h"
#include "steering_output.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;
static SteeringOutputStorage s_storage;
static int16_t s_control_stalk_analog_output;
static int16_t s_control_stalk_digital_output;

static StatusCode prv_handle_output(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  bool *received = context;

  if (*received == false) {
    uint16_t control_stalk_analog_state = 0;
    uint16_t control_stalk_digital_state = 0;
    TEST_ASSERT_OK(
        CAN_UNPACK_STEERING_OUTPUT(msg, &control_stalk_analog_state, &control_stalk_digital_state));
    TEST_ASSERT_EQUAL_UINT(s_control_stalk_analog_output, control_stalk_analog_state);
    TEST_ASSERT_EQUAL_UINT(s_control_stalk_digital_output, control_stalk_digital_state);
    *received = true;
  }

  return STATUS_CODE_OK;
}

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
  can_add_filter(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT);

  steering_output_init(&s_storage, INPUT_EVENT_STEERING_WATCHDOG_FAULT,
                       INPUT_EVENT_STEERING_UPDATE_REQUESTED);
}

void teardown_test(void) {}

void test_steering_output_working(void) {
  steering_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < NUM_STEERING_OUTPUT_SOURCES; i++) {
    steering_output_update(&s_storage, i, i * 100);
  }

  delay_ms(STEERING_OUTPUT_WATCHDOG_MS);

  // clear the event queue
  Event a;
  while (status_ok(event_process(&a))) {
  }

  // Should not have raised a fault event
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_UPDATE_REQUESTED, e.id);
  }

  steering_output_set_enabled(&s_storage, false);

  // Make sure that we don't raise any events after steering output has been disabled
  delay_ms(STEERING_OUTPUT_WATCHDOG_MS * 2);
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  // Reenable and fault
  steering_output_set_enabled(&s_storage, true);
  delay_ms(STEERING_OUTPUT_WATCHDOG_MS);
  ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_WATCHDOG_FAULT, e.id);
}

void test_steering_output_values(void) {
  // Test that s_storage is updated correctly
  s_control_stalk_analog_output = 70;
  s_control_stalk_digital_output = 80;

  steering_output_update(&s_storage, STEERING_OUTPUT_SOURCE_CONTROL_STALK_ANALOG_STATE,
                         s_control_stalk_analog_output);
  steering_output_update(&s_storage, STEERING_OUTPUT_SOURCE_CONTROL_STALK_DIGITAL_STATE,
                         s_control_stalk_digital_output);

  TEST_ASSERT_EQUAL(s_control_stalk_analog_output,
                    s_storage.data[STEERING_OUTPUT_SOURCE_CONTROL_STALK_ANALOG_STATE]);
  TEST_ASSERT_EQUAL(s_control_stalk_digital_output,
                    s_storage.data[STEERING_OUTPUT_SOURCE_CONTROL_STALK_DIGITAL_STATE]);

  // clear the event queue
  Event e;
  while (status_ok(event_process(&e))) {
  }

  // Test that the correct values are being transmitted over CAN
  bool received = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT, prv_handle_output, &received));
  steering_output_set_enabled(&s_storage, true);
  delay_ms(STEERING_OUTPUT_BROADCAST_MS);

  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_UPDATE_REQUESTED, e.id);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);
  TEST_ASSERT_TRUE(received);
}
