#include "can.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "event_queue.h"
#include "interrupt.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "exported_enums.h"
#include "lights_can.h"
#include "lights_events.h"

// Using SYSTEM_CAN_DEVICE_LIGHTS_REAR to be able to test sync message
#define TEST_LIGHTS_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_LIGHTS_REAR

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
  .generic_light_lookup = {
    [LIGHTS_GENERIC_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [LIGHTS_GENERIC_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [LIGHTS_GENERIC_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [LIGHTS_GENERIC_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
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
  TEST_ASSERT_OK(lights_can_init(&s_test_settings, &s_test_storage));
}

void teardown_test(void) {}

// Transmit a generic lights can message, and expect a GPIO event to be raised.
void test_lights_rx_generic_handler(void) {
  CANMessage on_msg = { 0 };
  TEST_ASSERT_OK(
      CAN_PACK_LIGHTS_GENERIC_TYPES(&on_msg, LIGHTS_GENERIC_TYPE_HIGH_BEAMS, LIGHTS_STATE_ON));
  TEST_ASSERT_OK(can_transmit(&on_msg, NULL));
  Event e = { 0 };
  prv_wait_tx_rx(&e);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS, e.data);

  CANMessage off_msg = { 0 };
  TEST_ASSERT_OK(
      CAN_PACK_LIGHTS_GENERIC_TYPES(&off_msg, LIGHTS_GENERIC_TYPE_LOW_BEAMS, LIGHTS_STATE_OFF));
  TEST_ASSERT_OK(can_transmit(&off_msg, NULL));
  prv_wait_tx_rx(&e);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS, e.data);
}

// Transmit a signal lights can message, and expect a signal event to be raised.
void test_lights_rx_signal_handler(void) {
  CANMessage sig_msg = { 0 };
  TEST_ASSERT_OK(
      CAN_PACK_LIGHTS_SIGNAL_STATES(&sig_msg, LIGHTS_EVENT_SIGNAL_MODE_LEFT, LIGHTS_STATE_ON));
  TEST_ASSERT_OK(can_transmit(&sig_msg, NULL));
  Event e = { 0 };
  prv_wait_tx_rx(&e);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SIGNAL_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SIGNAL_MODE_LEFT, e.data);
}

// Transmit a lights sync message and make sure sync event gets raised.
void test_lights_rx_sync_handler(void) {
  CANMessage sync_msg = { 0 };
  TEST_ASSERT_OK(CAN_PACK_LIGHTS_SYNC(&sync_msg));
  TEST_ASSERT_OK(can_transmit(&sync_msg, NULL));
  Event e = { 0 };
  prv_wait_tx_rx(&e);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
}

// Transmit a bps message and make sure lights bps event gets raised.
void test_lights_rx_bps_fault_handler(void) {
  CANMessage bps_msg = { 0 };
  TEST_ASSERT_OK(CAN_PACK_BPS_HEARTBEAT(&bps_msg, BPS_HEARTBEAT_STATE_OK));
  TEST_ASSERT_OK(can_transmit(&bps_msg, BPS_HEARTBEAT_STATE_OK));
  Event e = { 0 };
  prv_wait_tx_rx(&e);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_BPS, e.id);
  TEST_ASSERT_EQUAL(BPS_HEARTBEAT_STATE_OK, e.data);
}

// Transmit a horn message and make sure lights gpio event gets raised with horn as data.
void test_lights_rx_horn_handler(void) {
  CANMessage horn_msg = { 0 };
  TEST_ASSERT_OK(CAN_PACK_HORN(&horn_msg, HORN_STATE_ON));
  TEST_ASSERT_OK(can_transmit(&horn_msg, NULL));
  Event e = { 0 };
  prv_wait_tx_rx(&e);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HORN, e.data);
}
