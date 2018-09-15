#include "can.h"
#include "can_unpack.h"
#include "event_arbiter.h"
#include "exported_enums.h"
#include "hazards_fsm.h"
#include "headlight_fsm.h"
#include "horn_fsm.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "turn_signal_fsm.h"
#include "unity.h"

#define TEST_SIGNALS_MAX_

#define TEST_SIGNALS_CLOCK_EVENT(event_id, should_succeed)                                    \
  {                                                                                           \
    Event e = { .id = (event_id) };                                                           \
    TEST_ASSERT_EQUAL((should_succeed), event_arbiter_process_event(&s_arbiter_storage, &e)); \
  }

#define TEST_SIGNALS_EXPECT_LIGHT(light_id, light_state) \
  {                                                      \
    TEST_ASSERT_EQUAL((light_id), s_light_id);           \
    TEST_ASSERT_EQUAL((light_state), s_light_state);     \
  }

typedef struct TestSignalsExpectedLight {
  EELightType type;
  EELightState state;
} TestSignalsExpectedLight;

typedef enum {
  TEST_SIGNALS_FSM_HEADLIGHT = 0,
  TEST_SIGNALS_FSM_TURN_SIGNALS,
  TEST_SIGNALS_FSM_HAZARDS,
  TEST_SIGNALS_FSM_HORN,
  NUM_TEST_SIGNALS_FSMS,
} TestSignalsFsm;

EventArbiterStorage s_arbiter_storage;
static FSM s_fsms[NUM_TEST_SIGNALS_FSMS];
static CanStorage s_can_storage;
static EELightType s_light_id;
static EELightState s_light_state;

static void prv_dump_fsms(void) {
  for (size_t i = 0; i < NUM_TEST_SIGNALS_FSMS; i++) {
    printf("> %-30s%s\n", s_fsms[i].name, s_fsms[i].current_state->name);
  }
}

static void prv_clock_expected_lights(TestSignalsExpectedLight *lights, size_t num_lights) {
  uint8_t found_bitset = 0;
  uint8_t expected_bitset = (1 << num_lights) - 1;
  while (found_bitset != expected_bitset) {
    Event e = { 0 };
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT(can_process_event(&e));

    if (e.id == INPUT_EVENT_CAN_RX) {
      for (size_t j = 0; j < num_lights; j++) {
        if (lights[j].type == s_light_id) {
          TEST_ASSERT_EQUAL(lights[j].state, s_light_state);
          found_bitset |= 1 << j;
          break;
        }
      }
    }
  }
}

static StatusCode prv_light_state_cb(const CANMessage *msg, void *context,
                                     CANAckStatus *ack_reply) {
  uint8_t light_id = 0, state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &state);
  LOG_DEBUG("Light %d: %d\n", light_id, state);
  s_light_id = light_id;
  s_light_state = state;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
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

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_light_state_cb, NULL);

  event_arbiter_init(&s_arbiter_storage);
  turn_signal_fsm_init(&s_fsms[TEST_SIGNALS_FSM_TURN_SIGNALS], &s_arbiter_storage);
  horn_fsm_init(&s_fsms[TEST_SIGNALS_FSM_HORN], &s_arbiter_storage);
  hazards_fsm_init(&s_fsms[TEST_SIGNALS_FSM_HAZARDS], &s_arbiter_storage);
  headlight_fsm_init(&s_fsms[TEST_SIGNALS_FSM_HEADLIGHT], &s_arbiter_storage);
}

void teardown_test(void) {}

void test_signals_turn(void) {
  TestSignalsExpectedLight expected_lights[] = {
    { .type = EE_LIGHT_TYPE_SIGNAL_LEFT, .state = EE_LIGHT_STATE_ON },
    { .type = EE_LIGHT_TYPE_SIGNAL_RIGHT, .state = EE_LIGHT_STATE_OFF },
  };

  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT, true);
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  // New turn signal overrides previous
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT, true);
  expected_lights[0].state = EE_LIGHT_STATE_OFF;
  expected_lights[1].state = EE_LIGHT_STATE_ON;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  // Turning off the car turns off all lights
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_POWER_STATE_OFF, true);
  expected_lights[1].state = EE_LIGHT_STATE_OFF;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  prv_dump_fsms();
}

void test_signals_headlights(void) {
  TestSignalsExpectedLight expected_lights[] = {
    { .type = EE_LIGHT_TYPE_DRL, .state = EE_LIGHT_STATE_ON },
    { .type = EE_LIGHT_TYPE_LOW_BEAMS, .state = EE_LIGHT_STATE_OFF },
    { .type = EE_LIGHT_TYPE_HIGH_BEAMS, .state = EE_LIGHT_STATE_OFF },
  };

  LOG_DEBUG("Turning on DRLs\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DRL, true);
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Also turning on highbeams\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED, true);
  expected_lights[0].state = EE_LIGHT_STATE_OFF;
  expected_lights[2].state = EE_LIGHT_STATE_ON;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Turning off highbeams - revert to DRLs\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, true);
  expected_lights[0].state = EE_LIGHT_STATE_ON;
  expected_lights[2].state = EE_LIGHT_STATE_OFF;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Turning off DRLs\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DRL, true);
  expected_lights[0].state = EE_LIGHT_STATE_OFF;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Turning on highbeams\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED, true);
  expected_lights[2].state = EE_LIGHT_STATE_ON;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));
  prv_dump_fsms();

  LOG_DEBUG("Turning on lowbeams - highbeams stay on\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, true);
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Turning on DRLs - highbeams stay on\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DRL, true);
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Turning off highbeams - revert to DRLs\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED, true);
  expected_lights[0].state = EE_LIGHT_STATE_ON;
  expected_lights[2].state = EE_LIGHT_STATE_OFF;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("Switch to lowbeams\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS, true);
  expected_lights[0].state = EE_LIGHT_STATE_OFF;
  expected_lights[1].state = EE_LIGHT_STATE_ON;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));

  LOG_DEBUG("All lights off on fault\n");
  TEST_SIGNALS_CLOCK_EVENT(INPUT_EVENT_POWER_STATE_FAULT, true);
  expected_lights[1].state = EE_LIGHT_STATE_OFF;
  prv_clock_expected_lights(expected_lights, SIZEOF_ARRAY(expected_lights));
}
