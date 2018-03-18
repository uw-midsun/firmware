#include "can.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "event_queue.h"
#include "interrupt.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_can.h"
#include "lights_events.h"

#define NUM_TEST_MESSAGES_FRONT 6
#define NUM_TEST_MESSAGES_REAR 6
#define LIGHT_STATE_ON 6
#define LIGHT_STATE_OFF 6

#define CAN_RX_ADDR \
  { 0, 11 }

#define CAN_TX_ADDR \
  { 0, 12 }

static CANMessageID s_msg_id = 0x1;

// Waits for the internal RX/TX events to be sent and processes those events so that the next event
// is one raised by lights_can.c
static void prv_wait_tx_rx(Event *e) {
  int ret = NUM_STATUS_CODES;  // invalid status code
  while (ret != STATUS_CODE_OK) {
    ret = event_process(e);
    // wait
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_CAN_TX, e->id);
  fsm_process_event(CAN_FSM, e);  // process TX event
  ret = NUM_STATUS_CODES;
  while (ret != STATUS_CODE_OK) {
    ret = event_process(e);
    // wait
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_CAN_RX, e->id);
  fsm_process_event(CAN_FSM, e);  // process RX event
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

// sends messages to the front board using the loopback interface asserts that the correct event
// has been raised with the correct data.
void test_lights_rx_front(void) {
  const CANSettings can_settings_front = { .bitrate = CAN_HW_BITRATE_125KBPS,
                                           .rx_event = LIGHTS_EVENT_CAN_RX,
                                           .tx_event = LIGHTS_EVENT_CAN_TX,
                                           .fault_event = LIGHTS_EVENT_CAN_FAULT,
                                           .tx = CAN_TX_ADDR,
                                           .rx = CAN_RX_ADDR,
                                           .device_id = SYSTEM_CAN_DEVICE_LIGHTS_FRONT,
                                           .loopback = true };

  TEST_ASSERT_OK(lights_can_init(&can_settings_front));

  uint16_t test_messages_front[NUM_TEST_MESSAGES_FRONT][2] = {
    { LIGHTS_ACTION_SIGNAL_RIGHT, LIGHT_STATE_ON },   //
    { LIGHTS_ACTION_SIGNAL_LEFT, LIGHT_STATE_OFF },   //
    { LIGHTS_ACTION_SIGNAL_HAZARD, LIGHT_STATE_ON },  //
    { LIGHTS_ACTION_HORN, LIGHT_STATE_ON },           //
    { LIGHTS_ACTION_HEADLIGHTS, LIGHT_STATE_OFF },    //
    { LIGHTS_ACTION_SYNC, LIGHT_STATE_ON },           //
  };

  uint16_t assertion_values_front[NUM_TEST_MESSAGES_FRONT][2] = {
    { LIGHTS_EVENT_SIGNAL_RIGHT, LIGHT_STATE_ON },   //
    { LIGHTS_EVENT_SIGNAL_LEFT, LIGHT_STATE_OFF },   //
    { LIGHTS_EVENT_SIGNAL_HAZARD, LIGHT_STATE_ON },  //
    { LIGHTS_EVENT_HORN, LIGHT_STATE_ON },           //
    { LIGHTS_EVENT_HEADLIGHTS, LIGHT_STATE_OFF },    //
    { LIGHTS_EVENT_SYNC, LIGHT_STATE_ON }            //
  };

  CANMessage msg = { 0 };

  for (uint8_t i = 0; i < NUM_TEST_MESSAGES_FRONT; i++) {
    TEST_ASSERT_OK(
        CAN_PACK_LIGHTS_STATES(&msg, test_messages_front[i][0], test_messages_front[i][1]));
    TEST_ASSERT_OK(can_transmit(&msg, NULL));
    Event e = { 0 };
    prv_wait_tx_rx(&e);
    int ret = NUM_STATUS_CODES;
    while (ret != STATUS_CODE_OK) {
      ret = event_process(&e);
      // wait
    }
    TEST_ASSERT_EQUAL(assertion_values_front[i][0], e.id);
    TEST_ASSERT_EQUAL(assertion_values_front[i][1], e.data);
  }
}

// same as test_lights_rx_front, but for the rear board.
void test_lights_rx_rear(void) {
  const CANSettings can_settings_rear = { .bitrate = CAN_HW_BITRATE_125KBPS,
                                          .rx_event = LIGHTS_EVENT_CAN_RX,
                                          .tx_event = LIGHTS_EVENT_CAN_TX,
                                          .fault_event = LIGHTS_EVENT_CAN_FAULT,
                                          .tx = CAN_TX_ADDR,
                                          .rx = CAN_RX_ADDR,
                                          .device_id = SYSTEM_CAN_DEVICE_LIGHTS_REAR,
                                          .loopback = true };

  TEST_ASSERT_OK(lights_can_init(&can_settings_rear));

  uint16_t test_messages_rear[NUM_TEST_MESSAGES_REAR][2] = {
    { LIGHTS_ACTION_SIGNAL_RIGHT, LIGHT_STATE_OFF },  //
    { LIGHTS_ACTION_SIGNAL_LEFT, LIGHT_STATE_OFF },   //
    { LIGHTS_ACTION_SIGNAL_HAZARD, LIGHT_STATE_ON },  //
    { LIGHTS_ACTION_BRAKES, LIGHT_STATE_OFF },        //
    { LIGHTS_ACTION_STROBE, LIGHT_STATE_ON },         //
    { LIGHTS_ACTION_SYNC, LIGHT_STATE_ON },           //
  };

  uint16_t assertion_values_rear[NUM_TEST_MESSAGES_REAR][2] = {
    { LIGHTS_EVENT_SIGNAL_RIGHT, LIGHT_STATE_OFF },  //
    { LIGHTS_EVENT_SIGNAL_LEFT, LIGHT_STATE_OFF },   //
    { LIGHTS_EVENT_SIGNAL_HAZARD, LIGHT_STATE_ON },  //
    { LIGHTS_EVENT_BRAKES, LIGHT_STATE_OFF },        //
    { LIGHTS_EVENT_STROBE, LIGHT_STATE_ON },         //
    { LIGHTS_EVENT_SYNC, LIGHT_STATE_ON }            //
  };

  CANMessage msg = { 0 };

  for (uint8_t i = 0; i < NUM_TEST_MESSAGES_REAR; i++) {
    TEST_ASSERT_OK(
        CAN_PACK_LIGHTS_STATES(&msg, test_messages_rear[i][0], test_messages_rear[i][1]));
    TEST_ASSERT_OK(can_transmit(&msg, NULL));
    Event e = { 0 };
    prv_wait_tx_rx(&e);
    int ret = NUM_STATUS_CODES;
    while (ret != STATUS_CODE_OK) {
      ret = event_process(&e);
      // wait
    }
    TEST_ASSERT_EQUAL(assertion_values_rear[i][0], e.id);
    TEST_ASSERT_EQUAL(assertion_values_rear[i][1], e.data);
  }
}
