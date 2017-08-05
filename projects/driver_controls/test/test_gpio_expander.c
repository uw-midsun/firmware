#include "unity.h"
#include "log.h"
#include "test_helpers.h"
#include "gpio_expander.h"
#include "i2c.h"

void prv_callback(GPIOExpanderPin pin, GPIOState state, void *context) { }

void setup_test(void) {
  // GPIO initialization
  gpio_init();
  interrupt_init();
  gpio_it_init();

  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 }
  };

  i2c_init(I2C_PORT_1, &settings);

  GPIOAddress address = { GPIO_PORT_A, 0 };
  TEST_ASSERT_OK(gpio_expander_init(address, I2C_PORT_1));
}

void teardown_test(void) { }

void test_gpio_expander_init_pin(void) {
  GPIOSettings input_settings = { .direction = GPIO_DIR_IN };
  GPIOSettings output_settings = { .direction = GPIO_DIR_OUT, .state = GPIO_STATE_HIGH };
  TEST_ASSERT_NOT_OK(gpio_expander_init_pin(NUM_GPIO_EXPANDER_PINS, &output_settings));

  TEST_ASSERT_OK(gpio_expander_init_pin(GPIO_EXPANDER_PIN_0, &output_settings));
  TEST_ASSERT_OK(gpio_expander_init_pin(GPIO_EXPANDER_PIN_1, &input_settings));
}

void test_gpio_expander_register_callback(void) {
  TEST_ASSERT_NOT_OK(gpio_expander_register_callback(NUM_GPIO_EXPANDER_PINS, callback, NULL));

  TEST_ASSERT_OK(gpio_expander_register_callback(GPIO_EXPANDER_PIN_0, callback, NULL));
  TEST_ASSERT_OK(gpio_expander_register_callback(GPIO_EXPANDER_PIN_1, callback, NULL));
}

void test_gpio_expander_get_state(void) {
  GPIOState state = GPIO_STATE_LOW;
  TEST_ASSERT_NOT_OK(gpio_expander_get_state(NUM_GPIO_EXPANDER_PINS, &state));
  TEST_ASSERT_OK(gpio_expander_get_state(GPIO_EXPANDER_PIN_0, &state));
}

void test_gpio_expander_set_state(void) {
  GPIOState state = GPIO_STATE_LOW;
  TEST_ASSERT_NOT_OK(gpio_expander_set_state(NUM_GPIO_EXPANDER_PINS, state));
  TEST_ASSERT_NOT_OK(gpio_expander_set_state(NUM_GPIO_EXPANDER_PINS, state));
}
