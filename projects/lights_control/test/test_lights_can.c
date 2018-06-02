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

const LightsCanSettings s_test_settings = {
  // clang-format off
  .event_type = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_DRL] = LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_BRAKES] = LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_SIGNAL_RIGHT] = LIGHTS_CAN_EVENT_TYPE_SIGNAL,
    [EE_LIGHT_TYPE_SIGNAL_LEFT] = LIGHTS_CAN_EVENT_TYPE_SIGNAL,
    [EE_LIGHT_TYPE_SIGNAL_HAZARD] = LIGHTS_CAN_EVENT_TYPE_SIGNAL,
    [EE_LIGHT_TYPE_STROBE] = LIGHTS_CAN_EVENT_TYPE_STROBE,
  },
  .event_data_lookup = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [EE_LIGHT_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [EE_LIGHT_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
    [EE_LIGHT_TYPE_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_MODE_RIGHT,
    [EE_LIGHT_TYPE_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_MODE_LEFT,
    [EE_LIGHT_TYPE_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_MODE_HAZARD
  },
  // clang-format on
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .rx_addr = { .port = GPIO_PORT_A, .pin = 11 },
  .tx_addr = { .port = GPIO_PORT_A, .pin = 12 },
  .device_id = LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT
};

static LightsCanStorage s_storage = { 0 };
void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  lights_can_init(&s_storage, &s_test_settings);
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
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
}

// Transmit a sync message.
void test_lights_process_event(void) {
  const Event e = { .id = LIGHTS_EVENT_SYNC, .data = 0 };
  TEST_ASSERT_OK(lights_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(LIGHTS_EVENT_CAN_TX, LIGHTS_EVENT_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
}
