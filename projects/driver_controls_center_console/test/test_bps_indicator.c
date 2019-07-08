#include "bps_indicator.h"

#include "center_console_event.h"
#include "config.h"

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"

#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#include "exported_enums.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_storage = { 0 };

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  // GPIO initialization
  gpio_init();
  interrupt_init();
  gpio_it_init();

  CanSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .rx_event = CENTER_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTER_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTER_CONSOLE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  can_init(&s_storage, &settings);

  TEST_ASSERT_OK(bps_indicator_init());
}

void teardown_test(void) {}

void test_bps_indicator_raises_off_when_strobe_off(void) {
  // Send a mock CAN Lights State Message
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;

  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_STROBE, EE_LIGHT_STATE_OFF);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  // No events should be raised
  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_BPS, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_bps_indicator_raises_on_when_strobe_on(void) {
  // Send a mock CAN Lights State Message
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;

  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_STROBE, EE_LIGHT_STATE_ON);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  // No events should be raised
  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_BPS, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
