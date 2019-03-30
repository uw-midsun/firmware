#include "can.h"
#include "can_transmit.h"
#include "sc_cfg.h"
#include "sc_input_event.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_state_indicator.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = SC_CFG_CAN_DEVICE_ID,
    .bitrate = SC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_STEERING_CAN_RX,
    .tx_event = INPUT_EVENT_STEERING_CAN_TX,
    .fault_event = INPUT_EVENT_STEERING_CAN_FAULT,
    .tx = SC_CFG_CAN_RX,
    .rx = SC_CFG_CAN_TX,
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  can_add_filter(SYSTEM_CAN_MESSAGE_POWER_STATE);

  TEST_ASSERT_OK(power_state_indicator_init());
}

void teardown_test(void) {}

void test_power_state_idle(void) {
  uint8_t power_state = EE_POWER_STATE_IDLE;
  CAN_TRANSMIT_POWER_STATE(NULL, power_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_POWER_STATE_OFF, e.id);
}

void test_power_state_charge(void) {
  uint8_t power_state = EE_POWER_STATE_CHARGE;
  CAN_TRANSMIT_POWER_STATE(NULL, power_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_POWER_STATE_CHARGE, e.id);
}

void test_power_state_drive(void) {
  uint8_t power_state = EE_POWER_STATE_DRIVE;
  CAN_TRANSMIT_POWER_STATE(NULL, power_state);

  MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_STEERING_CAN_TX, INPUT_EVENT_STEERING_CAN_RX);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_STEERING_POWER_STATE_DRIVE, e.id);
}
