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

// Mock peripheral gpio peripheral definitions.
static const LightsGPIOPeripheral s_mock_peripherals[] = {
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_1] =
      {
          //
          .address = { .port = GPIO_PORT_A, .pin = 0 },  //
          .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
      },                                                 //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_2] =
      {
          //
          .address = { .port = GPIO_PORT_B, .pin = 0 },  //
          .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
      },                                                 //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_3] =
      {
          //
          .address = { .port = GPIO_PORT_A, .pin = 4 },  //
          .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,  //
      },                                                 //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_4] =
      {
          //
          .address = { .port = GPIO_PORT_B, .pin = 1 },  //
          .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,  //
      },                                                 //
  [TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_5] =
      {
          //
          .address = { .port = GPIO_PORT_A, .pin = 3 },  //
          .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
      },                                                 //
};

// Array of event-to-peripheral-set mappings.
static const LightsGPIOEventMapping s_mock_event_mappings[] = {
  { .event_id = TEST_LIGHTS_GPIO_MOCK_EVENT_1,  //
    .peripheral_mapping =                       //
    LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_1) |
    LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_3) },
  { .event_id = TEST_LIGHTS_GPIO_MOCK_EVENT_2,                         //
    .peripheral_mapping =                                              //
    LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_2) },  //
  { .event_id = TEST_LIGHTS_GPIO_MOCK_EVENT_3,                         //
    .peripheral_mapping =                                              //
    LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_4) |
    LIGHTS_GPIO_PERIPHERAL_BIT(TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_5) }  //
};

static const LightsGPIO s_mock_config = {
  .peripherals = s_mock_peripherals,                          //
  .num_peripherals = SIZEOF_ARRAY(s_mock_peripherals),        //
  .event_mappings = s_mock_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_mock_event_mappings),  //
};

// Asserts that the gpio state for all peripherals is initialized to high.
static void prv_gpio_initialized_high(const LightsGPIOPeripheral *peripherals, uint8_t size) {
  GPIOState state;
  for (uint8_t i = 0; i < size; i++) {
    TEST_ASSERT_OK(gpio_get_state(&peripherals[i].address, &state));
    TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);
  }
}

void setup_test(void) {
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(lights_gpio_init(&s_mock_config));
}

void teardown_test(void) {}

void test_lights_gpio_init(void) {
  prv_gpio_initialized_high(s_mock_peripherals, SIZEOF_ARRAY(s_mock_peripherals));
}

void test_lights_gpio_set_invalid_unsupported_event(void) {
  const Event invalid_event = { .id = NUM_TEST_LIGHTS_GPIO_MOCK_EVENTS, .data = 0 };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_config, &invalid_event));
  const Event unsupported_event = { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_4_UNSUPPORTED, .data = 1 };
  TEST_ASSERT_NOT_OK(lights_gpio_process_event(&s_mock_config, &unsupported_event));
}

void test_lights_gpio_process_event_1(void) {
  // As can be seen in s_mock_event_mappings, event 1 is associated with peripherals 1 and 3.
  const Event test_event = { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_1, .data = LIGHTS_GPIO_STATE_ON };
  TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &test_event));
  GPIOState gpio_state;
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_peripherals[TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_1].address, &gpio_state));
  TEST_ASSERT_EQUAL(gpio_state, GPIO_STATE_LOW);
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_peripherals[TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_3].address, &gpio_state));
  TEST_ASSERT_EQUAL(gpio_state, GPIO_STATE_HIGH);
}

void test_lights_gpio_process_event_2(void) {
  // As in s_mock_event_mappings, event 2 is associated with peripheral 2.
  const Event test_event = { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_1, .data = LIGHTS_GPIO_STATE_OFF };
  TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &test_event));
  GPIOState gpio_state;
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_peripherals[TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_2].address, &gpio_state));
  TEST_ASSERT_EQUAL(gpio_state, GPIO_STATE_HIGH);
}

void test_lights_gpio_process_event_3(void) {
  // As can be seen in s_mock_event_mappings, event 1 is associated with peripherals 1 and 3.
  const Event test_event = { .id = TEST_LIGHTS_GPIO_MOCK_EVENT_3, .data = LIGHTS_GPIO_STATE_OFF };
  TEST_ASSERT_OK(lights_gpio_process_event(&s_mock_config, &test_event));
  GPIOState gpio_state;
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_peripherals[TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_4].address, &gpio_state));
  TEST_ASSERT_EQUAL(gpio_state, GPIO_STATE_LOW);
  TEST_ASSERT_OK(
      gpio_get_state(&s_mock_peripherals[TEST_LIGHTS_GPIO_MOCK_PERIPHERAL_5].address, &gpio_state));
  TEST_ASSERT_EQUAL(gpio_state, GPIO_STATE_HIGH);
}
