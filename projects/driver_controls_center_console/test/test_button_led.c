#include "button_led.h"

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

// CanAckRequestCb
static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  (void)device;
  (void)num_remaining;
  CanAckStatus *expected_status = context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_POWER_STATE, msg_id);
  TEST_ASSERT_EQUAL(*expected_status, status);
  return STATUS_CODE_OK;
}

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

  ButtonLedGpioExpanderPins expander_pins = {
    .bps_indicator = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_BPS,
    .power_indicator = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_POWER,
    .lights_drl = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRL,
    .lights_low_beams = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_LOW_BEAMS,
    .lights_hazards = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_HAZARDS,
  };
  TEST_ASSERT_OK(button_led_init(&s_expander, &expander_pins));
}

void teardown_test(void) {}

void test_button_led_raises_update_event_off_when_rx_power_state_msg_idle(void) {
  // Send a fake Power State message
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;

  CanAckRequest req = {
    .callback = prv_ack_callback,
    .context = &expected_status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE),
  };
  CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_IDLE);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER, e.data);

  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_off_when_rx_power_state_msg_charge(void) {
  // Send a fake Power State message
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;

  CanAckRequest req = {
    .callback = prv_ack_callback,
    .context = &expected_status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE),
  };
  CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_CHARGE);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER, e.data);

  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_on_when_rx_power_state_msg_drive(void) {
  // Send a fake Power State message
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;

  CanAckRequest req = {
    .callback = prv_ack_callback,
    .context = &expected_status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE),
  };
  CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER, e.data);

  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_when_rx_lights_state_drl_off(void) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_OFF);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_when_rx_lights_state_drl_on(void) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_DRL, EE_LIGHT_STATE_ON);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_when_rx_lights_state_low_beams_off(void) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_OFF);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_when_rx_lights_state_low_beams_on(void) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_LOW_BEAMS, EE_LIGHT_STATE_ON);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_when_rx_lights_state_hazards_off(void) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_HAZARD, EE_LIGHT_STATE_OFF);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_button_led_raises_update_event_when_rx_lights_state_hazards_on(void) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_HAZARD, EE_LIGHT_STATE_ON);
  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT_EQUAL(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, e.id);
  TEST_ASSERT_EQUAL(EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
