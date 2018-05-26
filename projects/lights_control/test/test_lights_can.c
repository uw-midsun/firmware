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

#define TEST_LIGHTS_CAN_RX_ADDR \
  { .port = 0, .pin = 0 }
#define TEST_LIGHTS_CAN_TX_ADDR \
  { .port = 0, .pin = 1 }

LightsCanStorage s_test_storage;

typedef enum {
  TEST_LIGHTS_CAN_CMD_OFF = 0,
  TEST_LIGHTS_CAN_CMD_ON,
  NUM_TEST_LIGHTS_CAN_CMDS
} TestLightsCanCmd;

const LightsCanSettings s_test_settings = {
  .loopback = true,
  .rx_addr = TEST_LIGHTS_CAN_RX_ADDR,
  .tx_addr = TEST_LIGHTS_CAN_TX_ADDR,
  // clang-format off
  .event_lookup = {
    [LIGHTS_CAN_ACTION_SIGNAL_RIGHT] = { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_ON },
    [LIGHTS_CAN_ACTION_SIGNAL_LEFT] = { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_ON },
    [LIGHTS_CAN_ACTION_SIGNAL_HAZARD] = { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_ON },
    [LIGHTS_CAN_ACTION_HORN] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [LIGHTS_CAN_ACTION_HIGH_BEAMS] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [LIGHTS_CAN_ACTION_LOW_BEAMS] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [LIGHTS_CAN_ACTION_DRL] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [LIGHTS_CAN_ACTION_BRAKES] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [LIGHTS_CAN_ACTION_STROBE] = { LIGHTS_EVENT_STROBE_OFF, LIGHTS_EVENT_STROBE_ON },
    [LIGHTS_CAN_ACTION_SYNC] = { LIGHTS_EVENT_SYNC , LIGHTS_EVENT_SYNC},
  },
  .event_data_lookup = {
    [LIGHTS_CAN_ACTION_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_MODE_RIGHT,
    [LIGHTS_CAN_ACTION_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_MODE_LEFT,
    [LIGHTS_CAN_ACTION_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_MODE_HAZARD,
    [LIGHTS_CAN_ACTION_HORN] = LIGHTS_EVENT_GPIO_PERIPHERAL_HORN,
    [LIGHTS_CAN_ACTION_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [LIGHTS_CAN_ACTION_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [LIGHTS_CAN_ACTION_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [LIGHTS_CAN_ACTION_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
    [LIGHTS_CAN_ACTION_STROBE] = 0,
    [LIGHTS_CAN_ACTION_SYNC] = 0,
  },
  // clang-format on
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

// Transmit a CAN message to the module and make sure the correct events get raised
void test_lights_rx(void) {
  TEST_ASSERT_OK(lights_can_init(&s_test_settings, &s_test_storage));

  uint8_t num_test_messages = 7;

  uint16_t test_messages[][2] = {
    { LIGHTS_CAN_ACTION_SIGNAL_RIGHT, TEST_LIGHTS_CAN_CMD_ON },
    { LIGHTS_CAN_ACTION_SIGNAL_HAZARD, TEST_LIGHTS_CAN_CMD_OFF },
    { LIGHTS_CAN_ACTION_HIGH_BEAMS, TEST_LIGHTS_CAN_CMD_ON },
    { LIGHTS_CAN_ACTION_BRAKES, TEST_LIGHTS_CAN_CMD_OFF },
    { LIGHTS_CAN_ACTION_STROBE, TEST_LIGHTS_CAN_CMD_ON },
    { LIGHTS_CAN_ACTION_STROBE, TEST_LIGHTS_CAN_CMD_OFF },
    { LIGHTS_CAN_ACTION_SYNC, 0 },
  };

  uint16_t assertion_values[][2] = {
    { LIGHTS_EVENT_SIGNAL_ON, LIGHTS_EVENT_SIGNAL_MODE_RIGHT },
    { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_MODE_HAZARD },
    { LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS },
    { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES },
    { LIGHTS_EVENT_STROBE_ON, 0 },
    { LIGHTS_EVENT_STROBE_OFF, 0 },
    { LIGHTS_EVENT_SYNC, 0 },
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
