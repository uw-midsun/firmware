#include "can.h"
#include "can_transmit.h"
#include "cruise.h"
#include "event_queue.h"
#include "input_event.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "ms_test_helpers.h"

#define TEST_CRUISE_DEVICE_ID 1
#define TEST_CRUISE_NUM_RX_HANDLERS 3

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[TEST_CRUISE_NUM_RX_HANDLERS];

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

  StatusCode ret =
      can_init(&can_settings, &s_can_storage, s_rx_handlers, TEST_CRUISE_NUM_RX_HANDLERS);
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
  TEST_ASSERT(target < cruise_get_target_cms(cruise));

  // We should never have a negative cruise target
  e.id = INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS;
  cruise_handle_event(cruise, &e);
  cruise_handle_event(cruise, &e);
  cruise_handle_event(cruise, &e);
  TEST_ASSERT_EQUAL(0, cruise_get_target_cms(cruise));
}

void test_cruise_can(void) {
  Event e = { .id = INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED };
  CruiseStorage *cruise = cruise_global();

  // Send motor velocity messages - hit "set" to use current speed and observe target speed
  CAN_TRANSMIT_MOTOR_VELOCITY(20, 10);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  cruise_handle_event(cruise, &e);
  TEST_ASSERT_EQUAL((20 + 10) / 2, cruise_get_target_cms(cruise));

  // Handle negative velocity properly
  CAN_TRANSMIT_MOTOR_VELOCITY((uint32_t)-10, 20);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  cruise_handle_event(cruise, &e);
  TEST_ASSERT_EQUAL((-10 + 20) / 2, cruise_get_target_cms(cruise));

  // If average velocity is negative, cap to 0 (ex. reversing)
  CAN_TRANSMIT_MOTOR_VELOCITY((uint32_t)-40, (uint32_t)-40);
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  cruise_handle_event(cruise, &e);
  TEST_ASSERT_EQUAL(0, cruise_get_target_cms(cruise));
}
