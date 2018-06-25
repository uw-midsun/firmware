#include "can.h"
#include "can_transmit.h"
#include "cruise.h"
#include "delay.h"
#include "event_queue.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CRUISE_DEVICE_ID 1

static CANStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = TEST_CRUISE_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  TEST_ASSERT_OK(ret);

  cruise_init(cruise_global());
}

void teardown_test(void) {}

void test_cruise_basic(void) {
  CruiseStorage *cruise = cruise_global();
  int16_t target = cruise_get_target_cms(cruise);

  // Target should now be higher
  Event e = { .id = INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS };
  cruise_handle_event(cruise, &e);
  delay_ms(5);
  TEST_ASSERT(target < cruise_get_target_cms(cruise));

  // We should never have a negative cruise target
  e.id = INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS;
  cruise_handle_event(cruise, &e);
  delay_ms(5);
  TEST_ASSERT_EQUAL(0, cruise_get_target_cms(cruise));
}

void test_cruise_can(void) {
  Event e = { 0 };
  Event event_set = { .id = INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED };
  CruiseStorage *cruise = cruise_global();

  // Send motor velocity messages - hit "set" to use current speed and observe target speed
  LOG_DEBUG("Positive velocity\n");
  CAN_TRANSMIT_MOTOR_VELOCITY(20, 10);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(cruise_handle_event(cruise, &event_set));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_SPEED_UPDATE, e.id);
  TEST_ASSERT_EQUAL((20 + 10) / 2, e.data);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL((20 + 10) / 2, cruise_get_target_cms(cruise));

  // Handle negative velocity properly
  LOG_DEBUG("Negative velocity (average positive)\n");
  CAN_TRANSMIT_MOTOR_VELOCITY((uint16_t)-10, 20);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(cruise_handle_event(cruise, &event_set));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_SPEED_UPDATE, e.id);
  TEST_ASSERT_EQUAL((-10 + 20) / 2, e.data);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL((-10 + 20) / 2, cruise_get_target_cms(cruise));

  // If average velocity is negative, cap to 0 (ex. reversing)
  LOG_DEBUG("Negative velocity (cap to 0)\n");
  CAN_TRANSMIT_MOTOR_VELOCITY((uint16_t)-40, (uint16_t)-40);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(cruise_handle_event(cruise, &event_set));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_SPEED_UPDATE, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(0, cruise_get_target_cms(cruise));
}

/*
// Functional test
void test_cruise_target(void) {
  CruiseStorage *cruise = cruise_global();
  TEST_ASSERT_OK(cruise_set_target_cms(cruise, 30 * 45)); // Set to some arbitrary initial value
  int16_t prev_target = cruise_get_target_cms(cruise);

  Event e = { .id = INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS };
  cruise_handle_event(cruise, &e);

  while (true) {
    if (cruise_get_target_cms(cruise) != prev_target) {
      prev_target = cruise_get_target_cms(cruise);
      LOG_DEBUG("target cruise %d cm/s (%d mph)\n", prev_target, prev_target / CRUISE_OFFSET_CMS);
    }

    if (prev_target == CRUISE_MAX_TARGET_CMS) {
      e.id = INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS;
      cruise_handle_event(cruise, &e);
    }

    if (prev_target == 0) {
      e.id = INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS;
      cruise_handle_event(cruise, &e);
    }
  }
}
*/
