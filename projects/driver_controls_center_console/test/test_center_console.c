#include "center_console.h"

#include "can.h"
#include "config.h"
#include "critical_section.h"
#include "gpio.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#include <string.h>

static CanStorage s_can_storage = { 0 };
static CenterConsoleStorage s_cc_storage = { 0 };

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CENTER_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTER_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTER_CONSOLE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  can_init(&s_can_storage, &can_settings);

  CenterConsoleStorage cc_storage = {
    .momentary_switch_lights_low_beam =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_LOW_BEAM,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,
        },
    .momentary_switch_lights_drl =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_DRL,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL,
        },
    .toggle_switch_lights_hazards =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_HAZARDS,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,
        },
    .radio_button_drive =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_DRIVE,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE,
        },
    .radio_button_neutral =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_NEUTRAL,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL,
        },
    .radio_button_reverse =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_REVERSE,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE,
        },
    .toggle_switch_power =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_POWER,      //
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER,  //
        },
  };

  // Do a memcpy so we don't point to uninitialized memory after the stack
  // frame gets popped.
  memcpy(&s_cc_storage, &cc_storage, sizeof(s_cc_storage));
  TEST_ASSERT_OK(center_console_init(&s_cc_storage));
}

void teardown_test(void) {
  //
}

static StatusCode prv_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  bool *block = (bool *)context;

  *block = true;
  return STATUS_CODE_OK;
}
void test_center_console_momentary_switch_low_beam_raises_can_msg_on_gpio_it(void) {
  bool interrupt_ran = false;
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_CENTER_CONSOLE_EVENT, prv_callback,
                                         &interrupt_ran));

  // Trigger a SW interrupt
  bool disabled = critical_section_start();
  TEST_ASSERT_FALSE(interrupt_ran);

  GpioAddress pin_low_beam = CENTER_CONSOLE_CONFIG_PIN_LOW_BEAM;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_low_beam));
  critical_section_end(disabled);

  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_TRUE(interrupt_ran);
}

void test_center_console_momentary_switch_drl_raises_can_msg_on_gpio_it(void) {
  bool interrupt_ran = false;
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_CENTER_CONSOLE_EVENT, prv_callback,
                                         &interrupt_ran));

  // Trigger a SW interrupt
  bool disabled = critical_section_start();
  TEST_ASSERT_FALSE(interrupt_ran);

  GpioAddress pin_drl = CENTER_CONSOLE_CONFIG_PIN_DRL;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_drl));
  critical_section_end(disabled);

  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_TRUE(interrupt_ran);
}

void test_center_console_toggle_switch_hazards_raises_can_msg_on_gpio_it(void) {
  bool interrupt_ran = false;
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_CENTER_CONSOLE_EVENT, prv_callback,
                                         &interrupt_ran));

  // Trigger a SW interrupt
  bool disabled = critical_section_start();
  TEST_ASSERT_FALSE(interrupt_ran);

  GpioAddress pin_hazards = CENTER_CONSOLE_CONFIG_PIN_HAZARDS;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_hazards));
  critical_section_end(disabled);

  MS_TEST_HELPER_CAN_TX_RX(CENTER_CONSOLE_EVENT_CAN_TX, CENTER_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_TRUE(interrupt_ran);
}
