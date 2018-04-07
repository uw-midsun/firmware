#include <stdint.h>

#include "event_queue.h"
#include "gpio.h"
#include "misc.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_config.h"
#include "lights_events.h"
#include "lights_gpio.h"

#define TEST_LIGHTS_GPIO_MAKE_MASK(num) ((1) << (num))

typedef enum {
  LIGHTS_ACTION_TURN_OFF = 0,  //
  LIGHTS_ACTION_TURN_ON,       //
  NUM_LIGHTS_ACTIONS           //
} LightsAction;

typedef enum {
  MOCK_EVENT_1 = 0,          //
  MOCK_EVENT_2,              //
  MOCK_EVENT_3,              //
  MOCK_EVENT_4_UNSUPPORTED,  //
  NUM_MOCK_EVENTS            //
} MockEvent;

typedef enum {
  MOCK_PERIPHERAL_1 = 0,  //
  MOCK_PERIPHERAL_2,      //
  MOCK_PERIPHERAL_3,      //
  MOCK_PERIPHERAL_4,      //
  MOCK_PERIPHERAL_5,      //
  NUM_MOCK_PERIPHERALS    //
} MockPeripheral;

static const GPIOAddress s_mock_addresses[] = {
  // some valid addresses
  [MOCK_PERIPHERAL_1] = { .port = GPIO_PORT_A, .pin = 0 },  //
  [MOCK_PERIPHERAL_2] = { .port = GPIO_PORT_B, .pin = 0 },  //
  [MOCK_PERIPHERAL_3] = { .port = GPIO_PORT_A, .pin = 4 },  //
  [MOCK_PERIPHERAL_4] = { .port = GPIO_PORT_B, .pin = 1 },  //
  [MOCK_PERIPHERAL_5] = { .port = GPIO_PORT_A, .pin = 3 },  //
};

static const GPIOSettings s_mock_gpio_settings_out = {
  .direction = GPIO_DIR_OUT,       //
  .state = GPIO_STATE_HIGH,        //
  .resistor = GPIO_RES_NONE,       //
  .alt_function = GPIO_ALTFN_NONE  //
};

static const uint16_t s_mock_event_mappings[][2] = {
  { MOCK_EVENT_1, TEST_LIGHTS_GPIO_MAKE_MASK(MOCK_PERIPHERAL_1) |
                      TEST_LIGHTS_GPIO_MAKE_MASK(MOCK_PERIPHERAL_3) },  //
  { MOCK_EVENT_2, TEST_LIGHTS_GPIO_MAKE_MASK(MOCK_PERIPHERAL_2) },      //
  { MOCK_EVENT_3, TEST_LIGHTS_GPIO_MAKE_MASK(MOCK_PERIPHERAL_4) |
                      TEST_LIGHTS_GPIO_MAKE_MASK(MOCK_PERIPHERAL_5) }  //
};

static LightsConfig s_mock_conf = {
  .addresses = s_mock_addresses,                                //
  .num_addresses = SIZEOF_ARRAY(s_mock_addresses),              //
  .gpio_settings_out = &s_mock_gpio_settings_out,               //
  .event_mappings = s_mock_event_mappings,                      //
  .num_supported_events = SIZEOF_ARRAY(s_mock_event_mappings),  //
};

static void prv_gpio_initialized_high(const GPIOAddress *addrs, uint8_t size) {
  GPIOState state;
  for (uint8_t i = 0; i < size; i++) {
    TEST_ASSERT_OK(gpio_get_state(&addrs[i], &state));
    TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);
  }
}

static StatusCode prv_search_mappings_table(LightsConfig *conf, const Event *e, uint16_t *bitset) {
  for (uint8_t i = 0; i < conf->num_supported_events; i++) {
    if (e->id == conf->event_mappings[i][0]) {
      *bitset = conf->event_mappings[i][1];
      return STATUS_CODE_OK;
    }
  }
  return STATUS_CODE_INVALID_ARGS;
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

void setup_test(void) {
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(lights_gpio_init(&s_mock_conf));
}

void teardown_test(void) {}

void test_lights_gpio_init(void) {
  prv_gpio_initialized_high(s_mock_addresses, SIZEOF_ARRAY(s_mock_addresses));
}

void test_lights_gpio_set_invalid_unsupported_event(void) {
  const Event invalid_event = { .id = NUM_MOCK_EVENTS, .data = 0 };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_conf, &invalid_event));
  const Event unsupported_event = { .id = MOCK_EVENT_4_UNSUPPORTED, .data = 1 };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_conf, &unsupported_event));
}

void test_lights_gpio_process_event(void) {
  const Event test_events[] = {
    { .id = MOCK_EVENT_1, .data = LIGHTS_ACTION_TURN_ON },   //
    { .id = MOCK_EVENT_2, .data = LIGHTS_ACTION_TURN_ON },   //
    { .id = MOCK_EVENT_3, .data = LIGHTS_ACTION_TURN_OFF },  //
  };

  for (uint8_t i = 0; i < 3; i++) {
    TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_conf, &test_events[i]));
    uint16_t mapping_bitset = 0;
    TEST_ASSERT_OK(prv_search_mappings_table(&s_mock_conf, &test_events[i], &mapping_bitset));
    TEST_ASSERT_OK(prv_assert_gpio_state(mapping_bitset, !test_events[i].data, s_mock_addresses));
  }
}
