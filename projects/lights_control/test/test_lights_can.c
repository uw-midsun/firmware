#include "can.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "event_queue.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "can_transmit.h"
#include "exported_enums.h"
#include "lights_can.h"
#include "lights_can_config.h"
#include "lights_events.h"

static LightsCanStorage s_storage = { 0 };
void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  lights_can_config_init(LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT);
  LightsCanSettings *settings = lights_can_config_load();
  settings->loopback = true;
  lights_can_init(&s_storage, settings);
}

void teardown_test(void) {}

// Transmit a lights state can message, and expect the correct event to be raised.
void test_lights_rx_handler(void) {
  // Transmit an lights state ON message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHT_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_ON));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS, e.data);

  // Transmit an lights state OFF message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHT_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF));
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS, e.data);

  // Transmit a signal light ON message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHT_STATE(EE_LIGHT_TYPE_SIGNAL_RIGHT, EE_LIGHT_STATE_ON));
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SIGNAL_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SIGNAL_MODE_RIGHT, e.data);
}

// Transmit a bps heartbeat message and make sure lights bps heartbeat event gets raised.
void test_lights_rx_bps_heartbeat_handler(void) {
  TEST_ASSERT_OK(CAN_TRANSMIT_BPS_HEARTBEAT(NULL, EE_BPS_HEARTBEAT_STATE_OK));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_BPS_HEARTBEAT, e.id);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_OK, e.data);
}

// Transmit a horn message and make sure lights GPIO event gets raised with horn as data.
void test_lights_rx_horn_handler(void) {
  TEST_ASSERT_OK(CAN_TRANSMIT_HORN(EE_HORN_STATE_ON));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HORN, e.data);
}

// Transmit a sync message and make sure lights sync event gets raised.
void test_lights_sync_handler(void) {
  CANMessage msg_sync = { 0 };
  TEST_ASSERT_OK(CAN_PACK_LIGHTS_SYNC(&msg_sync));
  TEST_ASSERT_OK(can_transmit(&msg_sync, NULL));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
}
