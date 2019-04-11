#include "can.h"
#include "can_unpack.h"
#include "cc_cfg.h"
#include "cc_input_event.h"
#include "delay.h"
#include "drive_output.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

typedef struct {
  int16_t throttle;
  int16_t cruise;
  int16_t direction;
  int16_t mech_brake;
} TestDriveOutputData;

static CanStorage s_can_storage;
static DriveOutputStorage s_storage;
static TestDriveOutputData s_data;

static StatusCode prv_handle_output(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  bool *received = context;

  if (*received == false) {
    uint16_t throttle = 0;
    uint16_t direction = 0;
    uint16_t cruise = 0;
    uint16_t mech_brake = 0;
    TEST_ASSERT_OK(CAN_UNPACK_DRIVE_OUTPUT(msg, &throttle, &direction, &cruise, &mech_brake));
    TEST_ASSERT_EQUAL_UINT(s_data.throttle, throttle);
    TEST_ASSERT_EQUAL_UINT(s_data.direction, direction);
    TEST_ASSERT_EQUAL_UINT(s_data.cruise, cruise);
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
  can_add_filter(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT);

  drive_output_init(&s_storage, INPUT_EVENT_CENTER_CONSOLE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);
}

void teardown_test(void) {}

void test_drive_output_working(void) {
  // clear the event queue
  Event e;
  while (status_ok(event_process(&e))) {
  }

  drive_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < NUM_DRIVE_OUTPUT_SOURCES; i++) {
    drive_output_update(&s_storage, i, i * 100);
  }

  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // Should not have raised a fault event
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_NOT_EQUAL(INPUT_EVENT_CENTER_CONSOLE_WATCHDOG_FAULT, e.id);
  }

  drive_output_set_enabled(&s_storage, false);

  // Make sure that we don't raise any events after console output has been disabled
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS * 2);
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  // Reenable and fault
  drive_output_set_enabled(&s_storage, true);
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);
  ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(INPUT_EVENT_CENTER_CONSOLE_WATCHDOG_FAULT, e.id);
}

void test_drive_output_values(void) {
  // Test that s_storage is updated correctly
  s_data.throttle = 123;
  s_data.cruise = 45;
  s_data.direction = 2;
  s_data.mech_brake = 983;

  drive_output_update(&s_storage, DRIVE_OUTPUT_SOURCE_THROTTLE, s_data.throttle);
  drive_output_update(&s_storage, DRIVE_OUTPUT_SOURCE_CRUISE, s_data.cruise);
  drive_output_update(&s_storage, DRIVE_OUTPUT_SOURCE_DIRECTION, s_data.direction);
  drive_output_update(&s_storage, DRIVE_OUTPUT_SOURCE_MECH_BRAKE, s_data.mech_brake);

  TEST_ASSERT_EQUAL(s_data.throttle, s_storage.data[DRIVE_OUTPUT_SOURCE_THROTTLE]);
  TEST_ASSERT_EQUAL(s_data.cruise, s_storage.data[DRIVE_OUTPUT_SOURCE_CRUISE]);
  TEST_ASSERT_EQUAL(s_data.direction, s_storage.data[DRIVE_OUTPUT_SOURCE_DIRECTION]);
  TEST_ASSERT_EQUAL(s_data.mech_brake, s_storage.data[DRIVE_OUTPUT_SOURCE_MECH_BRAKE]);

  // clear the event queue
  Event e;
  while (status_ok(event_process(&e))) {
  }

  // Test that the correct values are being transmitted over CAN
  bool received = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_handle_output, &received));
  drive_output_set_enabled(&s_storage, true);
  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);

  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, e.id);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);
  TEST_ASSERT_TRUE(received);
}
