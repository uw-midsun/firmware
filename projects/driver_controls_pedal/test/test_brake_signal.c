#include "brake_signal.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "pedal_events.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Helper functions
#define TEST_BRAKE_SIGNAL_CLOCK_EVENT(event_id, event_data) \
  ({                                                        \
    Event _e = { .id = (event_id), .data = (event_data) };  \
    brake_signal_process_event(&_e);                        \
  })
#define TEST_BRAKE_SIGNAL_EXPECT_STATE(state)                         \
  ({                                                                  \
    MS_TEST_HELPER_CAN_TX_RX(PEDAL_EVENT_CAN_TX, PEDAL_EVENT_CAN_RX); \
    TEST_ASSERT_EQUAL((state), s_brake_state);                        \
  })

static CanStorage s_can_storage;
static EELightState s_brake_state = false;

// Callback used to observe the state of the Brake Signal
static StatusCode prv_light_state_cb(const CanMessage *msg, void *context,
                                     CanAckStatus *ack_reply) {
  uint8_t light_id = 0, state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &state);
  TEST_ASSERT_EQUAL(EE_LIGHT_TYPE_BRAKES, light_id);
  s_brake_state = state;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_EVENT_CAN_RX,
    .tx_event = PEDAL_EVENT_CAN_TX,
    .fault_event = PEDAL_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  TEST_ASSERT_OK(ret);

  s_brake_state = false;
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_light_state_cb, NULL);

  TEST_ASSERT_OK(brake_signal_init());
}

void teardown_test(void) {
  // Nothing
}

void test_brake_signal_process_event_should_process_power_fsm_event_power_state_off(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_OFF, 0));
}

void test_brake_signal_process_event_should_process_power_fsm_event_power_state_drive(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_DRIVE, 0));
}

void test_brake_signal_process_event_should_process_power_fsm_event_power_state_charge(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_CHARGE, 0));
}

void test_brake_signal_process_event_should_process_power_fsm_event_power_state_fault(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_FAULT, 0));
}

void test_brake_signal_process_event_should_process_mech_brake_fsm_event_brake_pressed(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED, 0));
}

void test_brake_signal_process_event_should_process_mech_brake_fsm_event_brake_released(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_RELEASED, 0));
}

void test_brake_signal_process_event_should_process_direction_fsm_event_neutral(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_DIRECTION_STATE_NEUTRAL, 0));
}

void test_brake_signal_process_event_should_process_direction_fsm_event_forward(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_DIRECTION_STATE_FORWARD, 0));
}

void test_brake_signal_process_event_should_process_direction_fsm_event_reverse(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_DIRECTION_STATE_REVERSE, 0));
}

void test_brake_signal_process_event_should_process_pedal_fsm_event_accel(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_PEDAL_ACCEL, 0));
}

void test_brake_signal_process_event_should_process_pedal_fsm_event_coast(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_PEDAL_COAST, 0));
}

void test_brake_signal_process_event_should_process_pedal_fsm_event_brake(void) {
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_PEDAL_BRAKE, 0));
}

void test_brake_signal_on_when_power_state_drive_and_mech_brake_engaged(void) {
  // Enter Drive State
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_DRIVE, 0));
  // Press mech brake
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_MECHANICAL_BRAKE_PRESSED, 0));

  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_ON);
}

void test_brake_signal_on_when_direction_forward(void) {
  // Enter Drive State
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_DRIVE, 0));
  // Set Direction to forwards
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_DIRECTION_STATE_FORWARD, 0));
  // Set speed to minimum threshold speed
  TEST_ASSERT_TRUE(
      TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_SPEED_UPDATE, BRAKE_SIGNAL_MIN_SPEED_CMS));
  // Move throttle to Brake zone
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_PEDAL_BRAKE, 0));

  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_ON);
}

void test_brake_signal_on_when_direction_reverse(void) {
  // Enter Drive State
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_POWER_STATE_DRIVE, 0));
  // Set Direction to reverse
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_DIRECTION_STATE_REVERSE, 0));
  // Set speed to minimum threshold speed
  TEST_ASSERT_TRUE(
      TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_SPEED_UPDATE, BRAKE_SIGNAL_MIN_SPEED_CMS));
  // Move throttle to Brake zone
  TEST_ASSERT_TRUE(TEST_BRAKE_SIGNAL_CLOCK_EVENT(PEDAL_EVENT_INPUT_PEDAL_BRAKE, 0));

  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_ON);
}
