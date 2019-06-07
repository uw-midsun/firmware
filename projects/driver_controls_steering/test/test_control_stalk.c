#include "control_stalk.h"

#include <string.h>

#include "can.h"
#include "can_msg_defs.h"
#include "config.h"
#include "critical_section.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "steering_events.h"
#include "test_helpers.h"

#include "unity.h"

static CanStorage s_can_storage = { 0 };
static ControlStalkStorage s_stalk = { 0 };

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = STEERING_EVENT_CAN_RX,
    .tx_event = STEERING_EVENT_CAN_TX,
    .fault_event = STEERING_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  can_init(&s_can_storage, &can_settings);

  // Enable Control Stalk
  ControlStalkStorage stalk = {
    .digital_config =
        {
            //
            [CONTROL_STALK_DIGITAL_INPUT_ID_HORN] =
                {
                    .pin = STEERING_CONFIG_PIN_HORN,
                    .can_event =
                        {
                            [GPIO_STATE_HIGH] = EE_STEERING_INPUT_HORN_PRESSED,  //
                            [GPIO_STATE_LOW] = EE_STEERING_INPUT_HORN_RELEASED   //
                        },
                },
            [CONTROL_STALK_DIGITAL_INPUT_ID_CC_ON_OFF] =
                {
                    .pin = STEERING_CONFIG_PIN_CC_ON_OFF,
                    .can_event =
                        {
                            [GPIO_STATE_HIGH] = EE_STEERING_INPUT_CC_ON_OFF_PRESSED,  //
                            [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_ON_OFF_RELEASED   //
                        },
                },
            [CONTROL_STALK_DIGITAL_INPUT_ID_SET] =
                {
                    .pin = STEERING_CONFIG_PIN_CC_SET,
                    .can_event =
                        {
                            [GPIO_STATE_HIGH] = EE_STEERING_INPUT_CC_SET_PRESSED,  //
                            [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_SET_RELEASED   //
                        },
                }  //
        },
    .analog_config =
        {
            //
            [CONTROL_STALK_ANALOG_INPUT_ID_CC_SPEED] =
                {
                    .address = STEERING_CONFIG_PIN_CC_SPEED,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_INPUT_CC_SPEED_NEUTRAL,  //
                            [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_INPUT_CC_SPEED_MINUS,    //
                            [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_INPUT_CC_SPEED_PLUS     //
                        },
                },
            [CONTROL_STALK_ANALOG_INPUT_ID_CC_CANCEL_RESUME] =
                {
                    .address = STEERING_CONFIG_PIN_CC_CANCEL_RESUME,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] =
                                EE_STEERING_INPUT_CC_CANCEL_RESUME_NEUTRAL,  //
                            [CONTROL_STALK_STATE_681_OHMS] =
                                EE_STEERING_INPUT_CC_CANCEL_RESUME_CANCEL,  //
                            [CONTROL_STALK_STATE_2181_OHMS] =
                                EE_STEERING_INPUT_CC_CANCEL_RESUME_RESUME  //
                        },
                },
            [CONTROL_STALK_ANALOG_INPUT_ID_TURN_SIGNAL_STALK] =
                {
                    .address = STEERING_CONFIG_PIN_TURN_SIGNAL_STALK,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] =
                                EE_STEERING_INPUT_TURN_SIGNAL_STALK_NONE,  //
                            [CONTROL_STALK_STATE_681_OHMS] =
                                EE_STEERING_INPUT_TURN_SIGNAL_STALK_RIGHT,  //
                            [CONTROL_STALK_STATE_2181_OHMS] =
                                EE_STEERING_INPUT_TURN_SIGNAL_STALK_LEFT  //
                        },
                },
            [CONTROL_STALK_ANALOG_INPUT_ID_CC_DISTANCE] =
                {
                    .address = STEERING_CONFIG_PIN_CC_DISTANCE,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] =
                                EE_STEERING_INPUT_EVENT_CC_DISTANCE_NEUTRAL,  //
                            [CONTROL_STALK_STATE_681_OHMS] =
                                EE_STEERING_INPUT_EVENT_CC_DISTANCE_MINUS,  //
                            [CONTROL_STALK_STATE_2181_OHMS] =
                                EE_STEERING_INPUT_EVENT_CC_DISTANCE_PLUS  //
                        },
                }  //
        },
  };
  // Do a memcpy so we don't point to uninitialized memory after the stack
  // frame gets popped.
  memcpy(&s_stalk, &stalk, sizeof(s_stalk));

  // x86 doesn't implement the ADC so we YOLO hack around it and just don't
  // check the return code, and don't exercise any ADC readings without
  // extensive mocking
  control_stalk_init(&s_stalk);
}

void teardown_test(void) {}

static StatusCode prv_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  bool *block = (bool *)context;

  *block = true;
  return STATUS_CODE_OK;
}

void test_control_stalk_raises_can_msg_when_digital_input_horn(void) {
  bool interrupt_ran = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_EVENT, prv_callback, &interrupt_ran));

  // Trigger a SW interrupt
  bool disabled = critical_section_start();
  TEST_ASSERT_FALSE(interrupt_ran);

  GpioAddress pin_horn = STEERING_CONFIG_PIN_HORN;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_horn));
  critical_section_end(disabled);

  MS_TEST_HELPER_CAN_TX_RX(STEERING_EVENT_CAN_TX, STEERING_EVENT_CAN_RX);

  TEST_ASSERT_TRUE(interrupt_ran);

  Event e = { 0 };
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_control_stalk_raises_can_msg_when_digital_input_cc_on_off(void) {
  bool interrupt_ran = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_EVENT, prv_callback, &interrupt_ran));

  // Trigger a SW interrupt
  bool disabled = critical_section_start();
  TEST_ASSERT_FALSE(interrupt_ran);

  GpioAddress pin_cc_on_off = STEERING_CONFIG_PIN_CC_ON_OFF;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_cc_on_off));
  critical_section_end(disabled);

  MS_TEST_HELPER_CAN_TX_RX(STEERING_EVENT_CAN_TX, STEERING_EVENT_CAN_RX);

  TEST_ASSERT_TRUE(interrupt_ran);

  Event e = { 0 };
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_control_stalk_raises_can_msg_when_digital_input_cc_set(void) {
  bool interrupt_ran = false;
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_EVENT, prv_callback, &interrupt_ran));

  // Trigger a SW interrupt
  bool disabled = critical_section_start();
  TEST_ASSERT_FALSE(interrupt_ran);

  GpioAddress pin_cc_set = STEERING_CONFIG_PIN_CC_SET;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_cc_set));
  critical_section_end(disabled);

  MS_TEST_HELPER_CAN_TX_RX(STEERING_EVENT_CAN_TX, STEERING_EVENT_CAN_RX);

  TEST_ASSERT_TRUE(interrupt_ran);

  Event e = { 0 };
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
