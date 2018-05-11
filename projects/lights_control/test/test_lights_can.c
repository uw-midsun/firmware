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

#define TEST_LIGHTS_CAN_DEVICE_ID 0

typedef enum {
  TEST_LIGHTS_CAN_CMD_OFF = 0,
  TEST_LIGHTS_CAN_CMD_ON,
  NUM_TEST_LIGHTS_CAN_CMDS
} TestLightsCanCmd;

LightsCanStorage s_test_storage;

const LightsCanSettings s_test_settings = {
  .loopback = true,
  // TODO(ELEC-372):  figure these out
  .rx_addr = { .port = 1, .pin = 1 },
  .tx_addr = { .port = 1, .pin = 1 },
  .event_lookup =
      {
          [LIGHTS_CAN_ACTION_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_RIGHT,
          [LIGHTS_CAN_ACTION_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_LEFT,
          [LIGHTS_CAN_ACTION_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_HAZARD,
          [LIGHTS_CAN_ACTION_HORN] = LIGHTS_EVENT_HORN,
          [LIGHTS_CAN_ACTION_HIGH_BEAMS] = LIGHTS_EVENT_HIGH_BEAMS,
          [LIGHTS_CAN_ACTION_LOW_BEAMS] = LIGHTS_EVENT_LOW_BEAMS,
          [LIGHTS_CAN_ACTION_DRL] = LIGHTS_EVENT_DRL,
          [LIGHTS_CAN_ACTION_BRAKES] = LIGHTS_EVENT_BRAKES,
          [LIGHTS_CAN_ACTION_STROBE] = LIGHTS_EVENT_STROBE,
          [LIGHTS_CAN_ACTION_SYNC] = LIGHTS_EVENT_SYNC,
      },
  .device_id = TEST_LIGHTS_CAN_DEVICE_ID
};

// Waits for the internal RX/TX events to be sent and processes those events so that the next event
// is one raised by lights_can module.
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
void test_lights_rx(void) {
  TEST_ASSERT_OK(lights_can_init(&s_test_settings, &s_test_storage));

  uint8_t num_test_messages = 5;

  uint16_t test_messages[][2] = {
    { LIGHTS_CAN_ACTION_SIGNAL_RIGHT, TEST_LIGHTS_CAN_CMD_ON },   //
    { LIGHTS_CAN_ACTION_SIGNAL_LEFT, TEST_LIGHTS_CAN_CMD_OFF },   //
    { LIGHTS_CAN_ACTION_SIGNAL_HAZARD, TEST_LIGHTS_CAN_CMD_ON },  //
    { LIGHTS_CAN_ACTION_HORN, TEST_LIGHTS_CAN_CMD_ON },           //
    { LIGHTS_CAN_ACTION_SYNC, TEST_LIGHTS_CAN_CMD_ON },           //
  };

  uint16_t assertion_values[][2] = {
    { LIGHTS_EVENT_SIGNAL_RIGHT, TEST_LIGHTS_CAN_CMD_ON },   //
    { LIGHTS_EVENT_SIGNAL_LEFT, TEST_LIGHTS_CAN_CMD_OFF },   //
    { LIGHTS_EVENT_SIGNAL_HAZARD, TEST_LIGHTS_CAN_CMD_ON },  //
    { LIGHTS_EVENT_HORN, TEST_LIGHTS_CAN_CMD_ON },           //
    { LIGHTS_EVENT_SYNC, TEST_LIGHTS_CAN_CMD_ON }            //
  };

  CANMessage msg = { 0 };

  for (uint8_t i = 0; i < num_test_messages; i++) {
    TEST_ASSERT_OK(CAN_PACK_LIGHTS_STATES(&msg, test_messages[i][0], test_messages[i][1]));
    TEST_ASSERT_OK(can_transmit(&msg, NULL));
    Event e = { 0 };
    prv_wait_tx_rx(&e);
    int ret = NUM_STATUS_CODES;
    while (ret != STATUS_CODE_OK) {
      ret = event_process(&e);
      // wait
    }
    TEST_ASSERT_EQUAL(assertion_values[i][0], e.id);
    TEST_ASSERT_EQUAL(assertion_values[i][1], e.data);
  }
}
