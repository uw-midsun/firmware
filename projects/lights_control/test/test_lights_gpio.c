#include <stdint.h>

#include "event_queue.h"
#include "gpio.h"
#include "misc.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_events.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"

// Creating a mock lights_gpio config.

// Mock events.
typedef enum {
  TEST_LIGHTS_GPIO_MOCK_EVENT_1 = 0,          //
  TEST_LIGHTS_GPIO_MOCK_EVENT_2,              //
  TEST_LIGHTS_GPIO_MOCK_EVENT_3,              //
  TEST_LIGHTS_GPIO_MOCK_EVENT_4_UNSUPPORTED,  //
  NUM_TEST_LIGHTS_GPIO_MOCK_EVENTS            //
} TestLightsGPIOMockEvent;

// Mock peripheral definitions.
typedef enum {
  TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_1 = 0,  //
  TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_2,      //
  TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_3,      //
  TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_4,      //
  TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_5,      //
  NUM_TEST_LIGHTS_GPIO_MOCK_PERIPHERALS    //
} TestLightsGPIOMockPeripheral;

// Mock peripheral gpio addresses.
static const GPIOAddress s_mock_addresses[] = {
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_1] = { .port = GPIO_PORT_A, .pin = 0 },  //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_2] = { .port = GPIO_PORT_B, .pin = 0 },  //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_3] = { .port = GPIO_PORT_A, .pin = 4 },  //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_4] = { .port = GPIO_PORT_B, .pin = 1 },  //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_5] = { .port = GPIO_PORT_A, .pin = 3 },  //
};

// Array of event-to-peripheral-set mappings.
static const LightsGPIOEventMapping s_mock_event_mappings[] = {
  { .event_id = TEST_LIGHTS_GPIO_MOCK_EVENT_1,
    .peripheral_mapping = LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_1) |
                      LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_3) },                            //
  { .event_id = TEST_LIGHTS_GPIO_MOCK_EVENT_2,
    .peripheral_mapping = LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_2) },  //
  { .event_id = TEST_LIGHTS_GPIO_MOCK_EVENT_3,
    .peripheral_mapping = LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_4) |
                      LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_5) }  //
};

static const LightsGPIO s_mock_config = {
  .peripherals = s_mock_addresses,                              //
  .num_peripherals = SIZEOF_ARRAY(s_mock_addresses),            //
  .event_mappings = s_mock_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_mock_event_mappings),  //
};

// Asserts that the gpio state for all peripherals is initialized to high.
static void prv_gpio_initialized_high(const GPIOAddress *addrs, uint8_t size) {
  GPIOState state;
  for (uint8_t i = 0; i < size; i++) {
    TEST_ASSERT_OK(gpio_get_state(&addrs[i], &state));
    TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);
  }
}

// Searches event-mappings for a mapping matching event e.
static StatusCode prv_search_mappings_table(LightsGPIO *conf, const Event *e, LightsGPIOPeripheralMapping *mapping) {
  for (uint8_t i = 0; i < conf->num_event_mappings; i++) {
    if (e->id == conf->event_mappings[i].event_id) {
      *mapping = conf->event_mappings[i].peripheral_mapping;
      return STATUS_CODE_OK;
    }
  }
  return STATUS_CODE_INVALID_ARGS;
}

// Asserts GPIO state of all peripherals in the mapping.
static StatusCode prv_assert_gpio_state(LightsGPIOPeripheralMapping mapping, GPIOState expected_state,
                                        const GPIOAddress *addresses) {
  uint8_t i;
  while (mapping) {
    i = __builtin_ffs(mapping) - 1;  // index of first 1 bit
    GPIOState current_state;
    TEST_ASSERT_OK(gpio_get_state(&addresses[i], &current_state));
    TEST_ASSERT_EQUAL(current_state, expected_state);
    mapping &= ~(1 << i);  // bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(lights_gpio_init(&s_mock_config));
}

void teardown_test(void) {}

void test_lights_gpio_init(void) {
  prv_gpio_initialized_high(s_mock_addresses, SIZEOF_ARRAY(s_mock_addresses));
}

void test_lights_gpio_set_invalid_unsupported_event(void) {
  const Event invalid_event = { .id = NUM_TEST_LIGHTS_GPIO_MOCK_EVENTS, .data = 0 };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_config, &invalid_event));
  const Event unsupported_event = { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_4_UNSUPPORTED, .data = 1 };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_config, &unsupported_event));
}

void test_lights_gpio_process_event(void) {
  const Event test_events[] = {
    { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_1, .data = LIGHTS_GPIO_STATE_ON },   //
    { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_2, .data = LIGHTS_GPIO_STATE_ON },   //
    { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_3, .data = LIGHTS_GPIO_STATE_OFF },  //
  };

  for (uint8_t i = 0; i < 3; i++) {
    TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &test_events[i]));
    uint16_t mapping_bitset = 0;
    TEST_ASSERT_OK(prv_search_mappings_table(&s_mock_config, &test_events[i], &mapping_bitset));
    TEST_ASSERT_OK(prv_assert_gpio_state(mapping_bitset, test_events[i].data, s_mock_addresses));
  }
}
