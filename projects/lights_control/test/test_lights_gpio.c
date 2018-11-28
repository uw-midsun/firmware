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

#define TEST_LIGHTS_GPIO_DUMMY_DATA 0

// Mock events.
typedef enum {
  TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERAL_1 = 0,  //
  TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERAL_2,      //
  NUM_TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERALS    //
} TestLightsGpioMockEventPeripheral;

// Mock output definitions.
typedef enum {
  TEST_LIGHTS_GPIO_MOCK_OUTPUT_1 = 0,  //
  TEST_LIGHTS_GPIO_MOCK_OUTPUT_2,      //
  TEST_LIGHTS_GPIO_MOCK_OUTPUT_3,      //
  TEST_LIGHTS_GPIO_MOCK_OUTPUT_4,      //
  NUM_TEST_LIGHTS_GPIO_MOCK_OUTPUTS    //
} TestLightsGpioMockOutput;

// Mock gpio output definitions.
static const LightsGpioOutput s_mock_outputs[] = {
  // clang-format off
  [TEST_LIGHTS_GPIO_MOCK_OUTPUT_1] = {
    .address = { .port = GPIO_PORT_A, .pin = 0 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [TEST_LIGHTS_GPIO_MOCK_OUTPUT_2] = {
    .address = { .port = GPIO_PORT_B, .pin = 0 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [TEST_LIGHTS_GPIO_MOCK_OUTPUT_3] = {
    .address = { .port = GPIO_PORT_A, .pin = 4 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [TEST_LIGHTS_GPIO_MOCK_OUTPUT_4] = {
    .address = { .port = GPIO_PORT_B, .pin = 1 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  }
  // clang-format on
};

// Array of event-to-peripheral-set mappings.
static const LightsGpioEventMapping s_mock_event_mappings[] = {
  { .peripheral = (LightsEventGpioPeripheral)TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERAL_1,  //
    .output_mapping =                                                                   //
    LIGHTS_GPIO_OUTPUT_BIT(TEST_LIGHTS_GPIO_MOCK_OUTPUT_1) |
    LIGHTS_GPIO_OUTPUT_BIT(TEST_LIGHTS_GPIO_MOCK_OUTPUT_3) },
  { .peripheral = (LightsEventGpioPeripheral)TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERAL_2,  //
    .output_mapping =                                                                   //
    LIGHTS_GPIO_OUTPUT_BIT(TEST_LIGHTS_GPIO_MOCK_OUTPUT_2) |
    LIGHTS_GPIO_OUTPUT_BIT(TEST_LIGHTS_GPIO_MOCK_OUTPUT_4) }
};

static const LightsGpio s_mock_config = {
  .outputs = s_mock_outputs,                                  //
  .num_outputs = SIZEOF_ARRAY(s_mock_outputs),                //
  .event_mappings = s_mock_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_mock_event_mappings),  //
};

void setup_test(void) {
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(lights_gpio_init(&s_mock_config));
}

void teardown_test(void) {}

// Tests that all lights are initialized to be turned off.
void test_lights_gpio_init(void) {
  GpioState state;
  for (uint8_t i = 0; i < s_mock_config.num_outputs; i++) {
    TEST_ASSERT_OK(gpio_get_state(&s_mock_config.outputs[i].address, &state));
    TEST_ASSERT_EQUAL((s_mock_config.outputs[i].polarity == LIGHTS_GPIO_POLARITY_ACTIVE_HIGH)
                          ? GPIO_STATE_LOW
                          : GPIO_STATE_HIGH,
                      state);
  }
}
void test_lights_gpio_ignore_unsupported_event(void) {
  // Unsupported event.
  const Event unsupported_event = { .id = LIGHTS_EVENT_CAN_RX,
                                    .data = TEST_LIGHTS_GPIO_DUMMY_DATA };
  // Ignore it with a STATUS_CODE_OK.
  TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &unsupported_event));
}

void test_lights_gpio_unsupported_peripheral(void) {
  // Event with an invalid (unsupported) data field.
  const Event invalid_event = { .id = LIGHTS_EVENT_GPIO_ON,
                                .data = NUM_TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERALS };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_config, &invalid_event));
}

// Testing that event with peripheral 1 is correctly mapped to outputs 1 and 3.
void test_lights_gpio_output_mapping(void) {
  const Event test_event = { .id = LIGHTS_EVENT_GPIO_ON,
                             .data = TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERAL_1 };
  TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &test_event));
  GpioState gpio_state;
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_outputs[TEST_LIGHTS_GPIO_MOCK_OUTPUT_1].address, &gpio_state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, gpio_state);
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_outputs[TEST_LIGHTS_GPIO_MOCK_OUTPUT_3].address, &gpio_state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, gpio_state);
}

// Testing that outputs are set with the correct corresponding polarity.
void test_lights_gpio_polarity_check(void) {
  const Event test_event = { .id = LIGHTS_EVENT_GPIO_ON,
                             .data = TEST_LIGHTS_GPIO_MOCK_EVENT_PERIPHERAL_2 };
  TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &test_event));
  GpioState gpio_state;
  // First output has polarity active low, we expect it to be low when turned ON.
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_outputs[TEST_LIGHTS_GPIO_MOCK_OUTPUT_2].address, &gpio_state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, gpio_state);
  // First output has polarity active high, we expect it to be high when turned ON.
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_outputs[TEST_LIGHTS_GPIO_MOCK_OUTPUT_4].address, &gpio_state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, gpio_state);
}
