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

const CanSettings s_can_settings = {
  // clang-format on
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx = { .port = GPIO_PORT_A, .pin = 11 },
  .tx = { .port = GPIO_PORT_A, .pin = 12 },
  .device_id = SYSTEM_CAN_DEVICE_LIGHTS_FRONT,
  .rx_event = LIGHTS_EVENT_CAN_RX,
  .tx_event = LIGHTS_EVENT_CAN_TX,
  .fault_event = LIGHTS_EVENT_CAN_FAULT
};

static LightsCanStorage s_storage = { 0 };

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  const LightsCanSettings *lights_settings = lights_can_config_load();
  lights_can_init(&s_storage, lights_settings, &s_can_settings);
}

void teardown_test(void) {}

// Transmit a lights state can message, and expect the correct event to be raised.
void test_lights_rx_handler(void) {
  // Transmit an lights state ON message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_ON));
  Event e = { 0 };
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS, e.data);

  // Transmit an lights state OFF message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF));
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS, e.data);

  // Transmit a signal light ON message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_RIGHT, EE_LIGHT_STATE_ON));
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SIGNAL_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SIGNAL_MODE_RIGHT, e.data);
}

// Transmit a horn message and make sure lights GPIO event gets raised with horn as data.
void test_lights_rx_horn_handler(void) {
  TEST_ASSERT_OK(CAN_TRANSMIT_HORN(EE_HORN_STATE_ON));
  Event e = { 0 };
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HORN, e.data);
}

// Transmit a sync message and make sure lights sync event gets raised.
void test_lights_sync_handler(void) {
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHTS_SYNC());
  Event e = { 0 };
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC_RX, e.id);
}

// Transmit a sync message.
void test_lights_process_event(void) {
  const Event e = { .id = LIGHTS_EVENT_SYNC_TX, .data = 0 };
  TEST_ASSERT_OK(lights_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC_RX, e.id);
}
