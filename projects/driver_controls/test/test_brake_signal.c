#include "brake_signal.h"
#include "can.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"
#include "exported_enums.h"
#include "ms_test_helpers.h"
#include "input_event.h"

#define TEST_BRAKE_SIGNAL_CLOCK_EVENT(event_id, event_data) \
  ({                                            \
    Event _e = { .id = (event_id), .data = (event_data) };  \
    brake_signal_process_event(&_e);            \
  })

#define TEST_BRAKE_SIGNAL_EXPECT_STATE(state)                         \
  ({                                                                  \
    MS_TEST_HELPER_CAN_TX_RX(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX); \
    TEST_ASSERT_EQUAL((state), s_brake_state);                        \
  })

static CANStorage s_can_storage;
static EELightState s_brake_state = false;

static StatusCode prv_light_state_cb(const CANMessage *msg, void *context,
                                     CANAckStatus *ack_reply) {
  uint8_t light_id = 0, state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &state);
  TEST_ASSERT_EQUAL(EE_LIGHT_TYPE_BRAKES, light_id);
  s_brake_state = state;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
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

void teardown_test(void) {}

void test_brake_signal_inputs(void) {
  Event e = { 0 };

  LOG_DEBUG("mech brake\n");
  // Mech brake should only turn on the brake signal if we enter drive
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, 0);
  // Nothing should have happened, so we shouldn't have any events
  TEST_ASSERT_NOT_OK(event_process(&e));

  LOG_DEBUG("enter drive\n");
  // Enter drive - expect brake light on
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_POWER_STATE_DRIVE, 0);
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, 0);
  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_ON);

  LOG_DEBUG("stop mech brake\n");
  // Stop mech braking - light should turn off
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, 0);
  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_OFF);

  // Try brake lights from regen braking
  // Regen braking by itself shouldn't cause the brake lights to turn on
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_PEDAL_BRAKE, 0);
  TEST_ASSERT_NOT_OK(event_process(&e));

  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, 0);
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_PEDAL_BRAKE, 0);
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_SPEED_UPDATE, BRAKE_SIGNAL_MIN_SPEED_CMS);
  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_ON);

  // Turn off the car and turn it back on
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_POWER_STATE_OFF, 0);
  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_OFF);
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_POWER_STATE_DRIVE, 0);

  // The direction is now neutral, so regen braking shouldn't cause the light to turn on
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_PEDAL_BRAKE, 0);
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_SPEED_UPDATE, BRAKE_SIGNAL_MIN_SPEED_CMS);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // Now move into drive
  TEST_BRAKE_SIGNAL_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, 0);
  TEST_BRAKE_SIGNAL_EXPECT_STATE(EE_LIGHT_STATE_ON);
}
