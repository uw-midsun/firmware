#include "button_led_radio.h"

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

// gpio_expander_init_pin mock to not have to pull in all I2C dependencies
StatusCode TEST_MOCK(gpio_expander_init_pin)(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                             const GpioSettings *settings) {
  return STATUS_CODE_OK;
}

static CanStorage s_storage = { 0 };
static GpioExpanderStorage s_expander = { 0 };

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

  // Initialize radio button groups
  ButtonLedRadioSettings radio_settings = {
    .reverse_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_REVERSE,
    .neutral_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_NEUTRAL,
    .drive_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRIVE,
  };
  TEST_ASSERT_OK(button_led_radio_init(&s_expander, &radio_settings));
}

void teardown_test(void) {}

void test_button_led_radio_raises_update_event_when_direction_forward(void) {
  CAN_TRANSMIT_DRIVE_OUTPUT(100, EE_DRIVE_OUTPUT_DIRECTION_FORWARD, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_DRIVE, e.id);
  TEST_ASSERT_EQUAL(0, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_radio_raises_update_event_when_direction_neutral(void) {
  CAN_TRANSMIT_DRIVE_OUTPUT(100, EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_NEUTRAL, e.id);
  TEST_ASSERT_EQUAL(0, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_radio_raises_update_event_when_direction_reverse(void) {
  CAN_TRANSMIT_DRIVE_OUTPUT(100, EE_DRIVE_OUTPUT_DIRECTION_REVERSE, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_REVERSE, e.id);
  TEST_ASSERT_EQUAL(0, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
