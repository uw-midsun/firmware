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
#include "lights_events.h"

// Using SYSTEM_CAN_DEVICE_LIGHTS_REAR to be able to test sync message. Rear board is the sender
// of the sync message.
#define TEST_LIGHTS_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_LIGHTS_REAR

LightsCanStorage s_test_storage;

typedef enum {
  TEST_LIGHTS_CAN_CMD_OFF = 0,
  TEST_LIGHTS_CAN_CMD_ON,
  NUM_TEST_LIGHTS_CAN_CMDS
} TestLightsCanCmd;

const LightsCanSettings s_test_settings = {
  .loopback = true,
  .rx_addr = { .port = GPIO_PORT_A, .pin = 11 },
  .tx_addr = { .port = GPIO_PORT_A, .pin = 12 },
  // clang-format off
  .peripheral_lookup = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [EE_LIGHT_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [EE_LIGHT_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
  },
  .device_id = TEST_LIGHTS_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_125KBPS
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  TEST_ASSERT_OK(lights_can_init(&s_test_storage, &s_test_settings));
}

void teardown_test(void) {}

// Transmit a lights state can message, and expect a GPIO event to be raised.
void test_lights_rx_handler(void) {
  // Transmit an lights state ON message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHT_STATE(EE_LIGHT_TYPE_HIGH_BEAMS, EE_LIGHT_STATE_ON));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {}
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS, e.data);

  // Transmit an lights state OFF message.
  TEST_ASSERT_OK(CAN_TRANSMIT_LIGHT_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF));
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {}
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS, e.data);
}

// Transmit a signal lights can message, and expect a signal event to be raised.
void test_lights_rx_signal_handler(void) {
  CANMessage sig_msg = { 0 };
  TEST_ASSERT_OK(
      CAN_PACK_SIGNAL_LIGHT_STATE(&sig_msg, LIGHTS_EVENT_SIGNAL_MODE_LEFT, EE_LIGHT_STATE_ON));
  TEST_ASSERT_OK(can_transmit(&sig_msg, NULL));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
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
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
}

// Transmit a bps message and make sure lights bps heartbeat event gets raised.
void test_lights_rx_bps_heartbeat_handler(void) {
  CANMessage bps_msg = { 0 };
  TEST_ASSERT_OK(CAN_PACK_BPS_HEARTBEAT(&bps_msg, EE_BPS_HEARTBEAT_STATE_OK));
  TEST_ASSERT_OK(can_transmit(&bps_msg, EE_BPS_HEARTBEAT_STATE_OK));
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
  CANMessage horn_msg = { 0 };
  TEST_ASSERT_OK(CAN_PACK_HORN(&horn_msg, EE_HORN_STATE_ON));
  TEST_ASSERT_OK(can_transmit(&horn_msg, NULL));
  Event e = { 0 };
  Event tx_event = { .id = LIGHTS_EVENT_CAN_TX, .data = 0 };
  Event rx_event = { .id = LIGHTS_EVENT_CAN_RX, .data = 0 };
  MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_HORN, e.data);
}
