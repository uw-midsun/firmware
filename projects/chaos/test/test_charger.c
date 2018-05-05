#include "charger.h"

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "gpio.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CHARGER_NUM_CAN_RX_HANDLERS 1

static CANStorage s_storage;
static CANRxHandler s_rx_handlers[TEST_CHARGER_NUM_CAN_RX_HANDLERS];
static ChargerState s_expected_state = NUM_CHARGER_STATES;

static void prv_charger_can_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  uint8_t state;
  CAN_UNPACK_CHARGING_PERMISSION(msg, &state);
  TEST_ASSERT
}

static void prv_transmit()

    void setup_test(void) {
  const CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .loopback = true,
  };

  can_init(&settings, &s_storage, s_rx_handlers, TEST_CHARGER_NUM_CAN_RX_HANDLERS);
  TEST_ASSERT_OK(charger_init());
}

void teardown_test(void) {}

void test_charger_state(void) {
  TEST_ASSERT_OK(charger_set_state(CHARGER_STATE_DISABLED));
  TEST_ASSERT_OK(charger_set_state(CHARGER_STATE_ENABLED));
  TEST_ASSERT_OK(charger_set_state(CHARGER_STATE_ENABLED));
}
