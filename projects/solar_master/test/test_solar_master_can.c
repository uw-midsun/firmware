#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#include "solar_master_can.h"
#include "solar_master_event.h"

static SolarMasterCanStorage s_solar_master_can_storage = { 0 };

static StatusCode prv_ack_request(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                  uint16_t remaining, void *context) {
  (void)context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT, device);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, status);
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, msg_id);
  TEST_ASSERT_EQUAL(0, remaining);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  CanSettings can_settings = {
    // clang-format on
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx = { .port = GPIO_PORT_A, .pin = 11 },
    .tx = { .port = GPIO_PORT_A, .pin = 12 },
    .device_id = SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT,
    .rx_event = SOLAR_MASTER_EVENT_CAN_RX,
    .tx_event = SOLAR_MASTER_EVENT_CAN_TX,
    .fault_event = SOLAR_MASTER_EVENT_CAN_FAULT,
  };

  TEST_ASSERT_OK(solar_master_can_init(&s_solar_master_can_storage, &can_settings,
                                       SOLAR_MASTER_CONFIG_BOARD_FRONT));
}

void teardown_test(void) {}

void test_solar_master_can_rx_handler(void) {
  const CanAckRequest ack_req = {
    .callback = prv_ack_request,
    .context = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT),
  };

  TEST_ASSERT_OK(CAN_TRANSMIT_SOLAR_RELAY_FRONT(&ack_req, EE_CHARGER_SET_RELAY_STATE_OPEN));
  // TX Relay Message.
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_MASTER_EVENT_CAN_TX, SOLAR_MASTER_EVENT_CAN_RX);
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(SOLAR_MASTER_EVENT_RELAY_STATE, e.id);
  TEST_ASSERT_EQUAL(EE_CHARGER_SET_RELAY_STATE_OPEN, e.data);
  // ACK Relay Message.
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_MASTER_EVENT_CAN_TX, SOLAR_MASTER_EVENT_CAN_RX);
}
