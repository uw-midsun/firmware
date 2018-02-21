#include "can.h"
#include "event_queue.h"
#include "interrupt.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_can.h"
#include "lights_gpio.h"
#include "lights_events.h"

// test for initializing the CAN settings

static CANMessageID s_msg_id = 0x1;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

//static void prv_clock_tx(Event* e) {
//  StatusCode ret;  
//  do {
//    ret = event_process(e);
//  } while (ret != STATUS_CODE_OK);
//  TEST_ASSERT_EQUAL(EVENT_CAN_TX, e->id);
//  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, e));
//  
//  do {
//    ret = event_process(e);
//  } while (ret != STATUS_CODE_OK);
//  TEST_ASSERT_EQUAL(EVENT_CAN_RX, e->id);
//  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, e));
//}

void test_lights_rx_front(void) {
  lights_can_init(LIGHTS_BOARD_FRONT);
  CANMessage msg = { 
    .msg_id = 0x1,
    .type = CAN_MSG_TYPE_DATA,
    .data = 0x4,
    .dlc = 2
  };
  uint16_t test_messages[] = { 0x0, 0x101, 0x002, 0x103, 0x004, 0x107 };

  uint16_t assertion_values[][2] = { { EVENT_SIGNAL_RIGHT, 0 },
                                     { EVENT_SIGNAL_LEFT, 1 },
                                     { EVENT_SIGNAL_HAZARD, 0 },
                                     { EVENT_HORN, 1 },
                                     { EVENT_HEADLIGHTS, 0 },
                                     { EVENT_SYNC, 1 } };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(test_messages); i++) {
    msg.data = test_messages[i];
    TEST_ASSERT_OK(can_transmit(&msg, NULL));
    Event e = { 0 };
    int ret;
    do {
      if (e.id == EVENT_CAN_RX) fsm_process_event(CAN_FSM, &e);
      if (e.id == EVENT_CAN_TX) fsm_process_event(CAN_FSM, &e);
      ret = event_process(&e);
    } while (ret != STATUS_CODE_OK || e.id == EVENT_CAN_RX || e.id == EVENT_CAN_TX);
    //prv_clock_tx(&e);
    //StatusCode ret;
    //do {
    //  ret = event_process(&e);
    //} while (ret != STATUS_CODE_OK);
    TEST_ASSERT_EQUAL(assertion_values[i][0], e.id);
    TEST_ASSERT_EQUAL(assertion_values[i][1], e.data);
  }
}

void test_lights_rx_rear(void) {
  lights_can_init(LIGHTS_BOARD_REAR);
  CANMessage msg = { 
    .msg_id = 0x1,
    .type = CAN_MSG_TYPE_DATA,
    .data = 0x4,
    .dlc = 2
  };
  uint16_t test_messages[] = { 0x0, 0x101, 0x002, 0x105, 0x006, 0x007 };

  uint16_t assertion_values[][2] = { { EVENT_SIGNAL_RIGHT, 0 },
                                     { EVENT_SIGNAL_LEFT, 1 },
                                     { EVENT_SIGNAL_HAZARD, 0 },
                                     { EVENT_BRAKES, 1 },
                                     { EVENT_STROBE, 0 },
                                     { EVENT_SYNC, 0 } };
  for (uint8_t i = 0; i < SIZEOF_ARRAY(test_messages); i++) {
    msg.data = test_messages[i];
    TEST_ASSERT_OK(can_transmit(&msg, NULL));
    Event e = { 0 };
    int ret;
    do {
      if (e.id == EVENT_CAN_RX) fsm_process_event(CAN_FSM, &e);
      if (e.id == EVENT_CAN_TX) fsm_process_event(CAN_FSM, &e);
      ret = event_process(&e);
    } while (ret != STATUS_CODE_OK || e.id == EVENT_CAN_RX || e.id == EVENT_CAN_TX);
    //prv_clock_tx(&e);
    //StatusCode ret;
    //do {
    //  ret = event_process(&e);
    //} while (ret != STATUS_CODE_OK);
    TEST_ASSERT_EQUAL(assertion_values[i][0], e.id);
    TEST_ASSERT_EQUAL(assertion_values[i][1], e.data);
  }
}

void test_lights_tx_sync(void) {
  lights_can_init(LIGHTS_BOARD_REAR);
  TEST_ASSERT_OK(send_sync());
  Event e = { 0 };
  int ret;
  do {
    if (e.id == EVENT_CAN_RX) fsm_process_event(CAN_FSM, &e);
    if (e.id == EVENT_CAN_TX) fsm_process_event(CAN_FSM, &e);
    ret = event_process(&e);
  } while (ret != STATUS_CODE_OK || e.id == EVENT_CAN_RX || e.id == EVENT_CAN_TX);
  TEST_ASSERT_EQUAL(EVENT_SYNC, e.id);
  TEST_ASSERT_EQUAL(1, e.data);
}

