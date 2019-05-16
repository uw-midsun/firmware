#include "can.h"
#include "can_unpack.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "sc_cfg.h"
#include "sc_input_event.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;
static int16_t s_control_stalk_analog_output;
static int16_t s_control_stalk_digital_output;

static StatusCode prv_handle_output(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  bool *received = context;

  if (*received == false) {
    uint16_t s_test_event = 0;
    uint16_t s_test_data = 0;
    TEST_ASSERT_OK(
        CAN_UNPACK_STEERING_OUTPUT(msg, &s_test_event, &s_test_data));
    TEST_ASSERT_EQUAL_UINT(s_test_event, s_test_data);
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

}

void teardown_test(void) {}

void test_steering_output_values(void) {
  // Test that s_storage is updated correctly
  uint16_t s_test_event = 4;
  uint16_t s_test_data = 80;
 

  CAN_TRANSMIT_STEERING_OUTPUT(s_test_event, s_test_data);

  // clear the event queue
  Event e;
  while (status_ok(event_process(&e))) {
  }

  // Test that the correct values are being transmitted over CAN
  bool received = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT, prv_handle_output, &received));

  MS_TEST_HELPER_AWAIT_EVENT(e);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);
  TEST_ASSERT_TRUE(received);
}
