#include "can.h"
#include "cruise.h"
#include "event_queue.h"
#include "input_event.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

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
    .rx_event = INPUT_EVENT_CAN_TX,
    .tx_event = INPUT_EVENT_CAN_RX,
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

  // Make sure trying to change the cruise target doesn't work if we're aren't currently in cruise
  cruise_set_source(cruise, CRUISE_SOURCE_MOTOR_CONTROLLER);
  int16_t target = cruise_get_target(cruise);
  Event e = { .id = INPUT_EVENT_CRUISE_CONTROL_INC };
  cruise_handle_event(cruise, &e);
  TEST_ASSERT_EQUAL(target, cruise_get_target(cruise));

  // Target should now be higher
  cruise_set_source(cruise, CRUISE_SOURCE_STORED_VALUE);
  e.id = INPUT_EVENT_CRUISE_CONTROL_INC;
  cruise_handle_event(cruise, &e);
  TEST_ASSERT(target < cruise_get_target(cruise));

  // We should never have a negative cruise target
  e.id = INPUT_EVENT_CRUISE_CONTROL_DEC;
  cruise_handle_event(cruise, &e);
  cruise_handle_event(cruise, &e);
  cruise_handle_event(cruise, &e);
  TEST_ASSERT_EQUAL(0, cruise_get_target(cruise));
}

void test_cruise_can(void) {
  // Send motor velocity messages
  // Also make sure that if the motor velocity is negative, we limit to 0
}
