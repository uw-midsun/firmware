#include "bps_indicator.h"
#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

static StatusCode prv_strobe_cb(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  EELightState *expected_state = context;

  uint8_t id = 0, state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &id, &state);
  TEST_ASSERT_EQUAL(EE_LIGHT_TYPE_STROBE, id);
  TEST_ASSERT_EQUAL(*expected_state, state);

  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = INPUT_EVENT_CENTER_CONSOLE_CAN_RX,
    .tx_event = INPUT_EVENT_CENTER_CONSOLE_CAN_TX,
    .fault_event = INPUT_EVENT_CENTER_CONSOLE_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  TEST_ASSERT_OK(ret);

  bps_indicator_init();
}

void teardown_test(void) {}

void test_bps_indicator_heartbeat(void) {
  Event e = { 0 };

  // Send BPS heartbeat good - make sure we receive an ACK and nothing happened
  CanAckRequest ack_request = {
    .callback = NULL,
    .context = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_DRIVER_CONTROLS),
  };
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_request, EE_BPS_HEARTBEAT_STATE_OK);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // Nothing should have happened, so no events should have been raised
  TEST_ASSERT_NOT_OK(event_process(&e));

  // Send BPS heartbeat not good
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_request, EE_BPS_HEARTBEAT_STATE_FAULT_KILLSWITCH);
  // Event is raised during ACK, so split up the processing
  // BPS heartbeat TX + RX
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // Raise BPS fault event
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_CENTER_CONSOLE_BPS_FAULT, e.id);

  // BPS heartbeat ACK TX/RX
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);
}

void test_bps_indicator_fault(void) {
  EELightState expected_state = NUM_EE_LIGHT_STATES;
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_strobe_cb, &expected_state);

  // Set fault - start strobes
  expected_state = EE_LIGHT_STATE_ON;
  TEST_ASSERT_OK(bps_indicator_set_fault());
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);

  // Clear fault - should have cleared strobe
  expected_state = EE_LIGHT_STATE_OFF;
  bps_indicator_clear_fault();
  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CENTER_CONSOLE_CAN_TX, INPUT_EVENT_CENTER_CONSOLE_CAN_RX);
}
