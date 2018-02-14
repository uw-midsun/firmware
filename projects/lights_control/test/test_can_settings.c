#include "can.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "can_setup.h"
#include "lights_events.h"
#include "structs.h"

// test for initializing the CAN settings

static CANMessageID s_msg_id = 0x1;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_can_rx_front(void) {
  can_setup_init(LIGHTS_BOARD_FRONT);
  CANMessage msg = { .msg_id = 0x1, .type = CAN_MSG_TYPE_DATA, .data = 0x4, .dlc = 2 };
  uint16_t test_messages[] = { 0x0, 0x101, 0x002, 0x103, 0x004 };
  uint16_t assertion_values[][2] = { { EVENT_SIGNAL_RIGHT, 0 },
                                     { EVENT_SIGNAL_LEFT, 1 },
                                     { EVENT_SIGNAL_HAZARD, 0 },
                                     { EVENT_HORN, 1 },
                                     { EVENT_HEADLIGHTS, 0 } };

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
    TEST_ASSERT_EQUAL(assertion_values[i][0], e.id);
    TEST_ASSERT_EQUAL(assertion_values[i][1], e.data);
  }
}
