#include <stdint.h>

#include "event_queue.h"
#include "gpio.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_config.h"
#include "lights_events.h"
#include "lights_gpio.h"

typedef enum { LIGHTS_ACTION_TURN_OFF = 0, LIGHTS_ACTION_TURN_ON, NUM_LIGHTS_ACTIONS } LightsAction;

void setup_test(void) {
  TEST_ASSERT_OK(gpio_init());
}

void teardown_test(void) {}

static void prv_gpio_initialized_high(const GPIOAddress *addrs, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    GPIOState state;
    TEST_ASSERT_OK(gpio_get_state(&addrs[i], &state));
    TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);
  }
}

static StatusCode prv_assert_gpio_state(uint16_t bitset, GPIOState expected_state,
                                        const GPIOAddress *addresses) {
  uint8_t i;
  while (bitset) {
    i = __builtin_ffs(bitset) - 1;  // index of first 1 bit
    GPIOState current_state;
    TEST_ASSERT_OK(gpio_get_state(&addresses[i], &current_state));
    TEST_ASSERT_EQUAL(current_state, expected_state);
    bitset &= ~(1 << i);  // bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

void test_get_lights_board_not_initialized() {
  LightsBoard board;
  TEST_ASSERT_NOT_OK(lights_gpio_get_lights_board(&board));
}

void test_lights_gpio_set_not_initialized() {
  const Event e = { .id = 0, .data = 0 };  // some valid event
  TEST_ASSERT_NOT_OK(lights_gpio_set(&e));
}

void test_lights_gpio_init_front() {
  LightsConfig *conf = lights_config_load();
  const GPIOAddress *front_addrs = conf->addresses_front;
  uint8_t front_addrs_num = conf->num_addresses_front;
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_FRONT));
  prv_gpio_initialized_high(front_addrs, front_addrs_num);
}

void test_lights_gpio_init_rear() {
  LightsConfig *conf = lights_config_load();
  const GPIOAddress *rear_addrs = conf->addresses_rear;
  uint8_t rear_addrs_num = conf->num_addresses_rear;
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_REAR));
  prv_gpio_initialized_high(rear_addrs, rear_addrs_num);
}

void test_lights_gpio_set_invalid_event_front() {
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_FRONT));
  const Event invalid_event = { .id = NUM_FRONT_LIGHTS_EVENTS, .data = 0 };
  TEST_ASSERT_NOT_OK(lights_gpio_set(&invalid_event));
}

void test_lights_gpio_set_front() {
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_FRONT));
  uint16_t *mapping = test_lights_gpio_event_mappings(LIGHTS_BOARD_FRONT);
  LightsConfig *conf = lights_config_load();
  const GPIOAddress *addresses_front = conf->addresses_front;

  const Event test_events[] = {
    { .id = FRONT_LIGHTS_EVENT_HORN, .data = LIGHTS_ACTION_TURN_ON },
    { .id = FRONT_LIGHTS_EVENT_HIGH_BEAMS, .data = LIGHTS_ACTION_TURN_ON },
    { .id = FRONT_LIGHTS_EVENT_LOW_BEAMS, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = FRONT_LIGHTS_EVENT_DRL, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = FRONT_LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = FRONT_LIGHTS_EVENT_SIGNAL_RIGHT, .data = LIGHTS_ACTION_TURN_ON },
    { .id = FRONT_LIGHTS_EVENT_SIGNAL_HAZARD, .data = LIGHTS_ACTION_TURN_ON },
    { .id = FRONT_LIGHTS_EVENT_SYNC, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = FRONT_LIGHTS_EVENT_CAN_RX, .data = LIGHTS_ACTION_TURN_ON },
    { .id = FRONT_LIGHTS_EVENT_CAN_TX, .data = LIGHTS_ACTION_TURN_ON },
    { .id = FRONT_LIGHTS_EVENT_CAN_FAULT, .data = LIGHTS_ACTION_TURN_ON },
  };

  for (uint8_t i = 0; i < NUM_FRONT_LIGHTS_EVENTS; i++) {
    TEST_ASSERT_OK(lights_gpio_set(&test_events[i]));
    TEST_ASSERT_OK(
        prv_assert_gpio_state(mapping[test_events[i].id], !test_events[i].data, addresses_front));
  }
}

void test_lights_gpio_set_invalid_event_rear() {
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_REAR));
  const Event invalid_event = { .id = NUM_REAR_LIGHTS_EVENTS, .data = 0 };
  TEST_ASSERT_NOT_OK(lights_gpio_set(&invalid_event));
}

void test_lights_gpio_set_rear() {
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_REAR));
  uint16_t *mapping = test_lights_gpio_event_mappings(LIGHTS_BOARD_REAR);
  LightsConfig *conf = lights_config_load();
  const GPIOAddress *addresses_rear = conf->addresses_rear;

  const Event test_events[] = {
    { .id = REAR_LIGHTS_EVENT_STROBE, .data = LIGHTS_ACTION_TURN_ON },
    { .id = REAR_LIGHTS_EVENT_BRAKES, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = REAR_LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_ACTION_TURN_ON },
    { .id = REAR_LIGHTS_EVENT_SIGNAL_RIGHT, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = REAR_LIGHTS_EVENT_SIGNAL_HAZARD, .data = LIGHTS_ACTION_TURN_OFF },
    { .id = REAR_LIGHTS_EVENT_CAN_RX, .data = LIGHTS_ACTION_TURN_ON },
    { .id = REAR_LIGHTS_EVENT_CAN_TX, .data = LIGHTS_ACTION_TURN_ON },
    { .id = REAR_LIGHTS_EVENT_CAN_FAULT, .data = LIGHTS_ACTION_TURN_OFF },
  };

  for (uint8_t i = 0; i < NUM_REAR_LIGHTS_EVENTS; i++) {
    TEST_ASSERT_OK(lights_gpio_set(&test_events[i]));
    TEST_ASSERT_OK(
        prv_assert_gpio_state(mapping[test_events[i].id], !test_events[i].data, addresses_rear));
  }
}

void test_get_lights_board_front() {
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_FRONT));
  LightsBoard board;
  TEST_ASSERT_OK(lights_gpio_get_lights_board(&board));
  TEST_ASSERT_EQUAL(board, LIGHTS_BOARD_FRONT);
}

void test_get_lights_board_rear() {
  TEST_ASSERT_OK(lights_gpio_init(LIGHTS_GPIO_INIT_MODE_TEST_REAR));
  LightsBoard board;
  TEST_ASSERT_OK(lights_gpio_get_lights_board(&board));
  TEST_ASSERT_EQUAL(board, LIGHTS_BOARD_REAR);
}
