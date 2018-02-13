#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "event_queue.h"
#include "interrupt.h"
#include "unity.h"
#include "can.h"

#include <stdio.h>

#include "can_settings.h"
#include "structs.h"
#include "lights_events.h"

// test for initializing the CAN settings

static BoardType s_boardtype = LIGHTS_BOARD_FRONT;
static CANMessageID s_msg_id = 0x1; 

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  initialize_can_settings(s_boardtype);
}

void teardown_test(void) {}

static void prv_clock_tx(void) {
  Event e = { 0 };
  StatusCode ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(EVENT_CAN_TX, e.id);
  bool processed = fsm_process_event(CAN_FSM, &e);
  TEST_ASSERT_TRUE(processed);
}

void test_can_message(void) {
  CANMessage msg = {
    .msg_id = 0x1,              
    .type = CAN_MSG_TYPE_DATA,
    .data = 0x4,               
    .dlc = 1,                   
  };

  // Begin CAN transmit request
  StatusCode ret = can_transmit(&msg, NULL);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  Event e = { 0 };

  // Wait for RX
  while (event_process(&e) != STATUS_CODE_OK) {}
  TEST_ASSERT_EQUAL(EVENT_CAN_RX, e.id);
  printf("receives the event\n")
  bool processed = fsm_process_event(CAN_FSM, &e);
  printf("but gets segfault here\n");
  TEST_ASSERT_TRUE(processed);

}

