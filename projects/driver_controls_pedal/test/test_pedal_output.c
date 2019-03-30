#include "can.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pc_cfg.h"
#include "pc_input_event.h"
#include "pedal_output.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

typedef struct {
  int16_t throttle;
  int16_t throttle_state;
  int16_t mech_brake;
} TestPedalOutputData;

static CanStorage s_can_storage;
static PedalOutputStorage s_storage;
static TestPedalOutputData s_data;

static StatusCode prv_handle_output(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  bool *received = context;

  if (*received == false) {
    uint16_t throttle = 0;
    uint16_t throttle_state = 0;
    uint16_t mech_brake = 0;
    TEST_ASSERT_OK(CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle, &throttle_state, &mech_brake));
    TEST_ASSERT_EQUAL_UINT(s_data.throttle, throttle);
    TEST_ASSERT_EQUAL_UINT(s_data.throttle_state, throttle_state);
    TEST_ASSERT_EQUAL_UINT(s_data.mech_brake, mech_brake);
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
  can_add_filter(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT);

  pedal_output_init(&s_storage, INPUT_EVENT_PEDAL_WATCHDOG_FAULT, INPUT_EVENT_PEDAL_UPDATE_REQUESTED);
}

void teardown_test(void) {}

void test_pedal_output_working(void) {
  pedal_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < NUM_PEDAL_OUTPUT_SOURCES; i++) {
    pedal_output_update(&s_storage, i, i * 100);
  }

  delay_ms(PEDAL_OUTPUT_WATCHDOG_MS);

  // Should not have raised a fault event
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_UPDATE_REQUESTED, e.id);
  }

  pedal_output_set_enabled(&s_storage, false);

  // Make sure that we don't raise any events after pedal output has been disabled
  delay_ms(PEDAL_OUTPUT_WATCHDOG_MS * 2);
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  // Reenable and fault
  pedal_output_set_enabled(&s_storage, true);
  delay_ms(PEDAL_OUTPUT_WATCHDOG_MS);
  ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_WATCHDOG_FAULT, e.id);
}

void test_pedal_output_values(void) {
  // Test that s_storage is updated correctly
  s_data.throttle = 49;
  s_data.throttle_state = 1;
  s_data.mech_brake = 763;

  pedal_output_update(&s_storage, PEDAL_OUTPUT_SOURCE_THROTTLE, s_data.throttle);
  pedal_output_update(&s_storage, PEDAL_OUTPUT_SOURCE_THROTTLE_STATE, s_data.throttle_state);
  pedal_output_update(&s_storage, PEDAL_OUTPUT_SOURCE_MECH_BRAKE, s_data.mech_brake);

  TEST_ASSERT_EQUAL(s_data.throttle, s_storage.data[PEDAL_OUTPUT_SOURCE_THROTTLE]);
  TEST_ASSERT_EQUAL(s_data.throttle_state, s_storage.data[PEDAL_OUTPUT_SOURCE_THROTTLE_STATE]);
  TEST_ASSERT_EQUAL(s_data.mech_brake, s_storage.data[PEDAL_OUTPUT_SOURCE_MECH_BRAKE]);

  // clear the event queue
  Event e;
  while (status_ok(event_process(&e))) {
  }

  // Test that the correct values are being transmitted over CAN
  bool received = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_output, &received));
  pedal_output_set_enabled(&s_storage, true);
  delay_ms(PEDAL_OUTPUT_BROADCAST_MS);

  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_UPDATE_REQUESTED, e.id);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_PEDAL_CAN_TX, INPUT_EVENT_PEDAL_CAN_RX);
  TEST_ASSERT_TRUE(received);
}
