#include "gpio_expander.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"
#include "center_console.h"
#include "wait.h"
#include "soft_timer.h"
#include "delay.h"



static void prv_pin_callback(GpioExpanderPin pin, GPIOState state, void *context) {
  CenterConsoleStorage *storage = context;
  LOG_DEBUG("Center Console - PIN: %d, STATE: %d\n", pin, state);
}

static GpioExpanderStorage s_console_expander;
//static GpioExpanderStorage s_stalk_expander;

// For now we only dump the centre console data.
static StatusCode prv_gpio_expander_dump_init(GpioExpanderStorage *expander) {
  const GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN };

  for (size_t i = 0; i < NUM_CENTER_CONSOLE_INPUTS; i++) {
    gpio_expander_init_pin(expander, i, &gpio_settings);
    gpio_expander_register_callback(expander, i, prv_pin_callback, NULL);
  }

  return STATUS_CODE_OK;
}

void setup_test(void) {
  // GPIO initialization
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = { GPIO_PORT_B, 9 },  //
    .scl = { GPIO_PORT_B, 8 },  //
  };

  i2c_init(I2C_PORT_1, &settings);

  GPIOAddress int_pin = { GPIO_PORT_A, 2 };

  // Initialize control stalk GPIO expander.
  gpio_expander_init(&s_console_expander, I2C_PORT_1, GPIO_EXPANDER_ADDRESS_0, &int_pin);

  // Initialize centre console GPIO expander.
  //TEST_ASSERT_OK(gpio_expander_init(&s_stalk_expander, I2C_PORT_1, GPIO_EXPANDER_ADDRESS_1, &int_pin));

  TEST_ASSERT_OK(prv_gpio_expander_dump_init(&s_console_expander));
}

void teardown_test(void) {}

void test_gpio_expander_dump(void) {
  LOG_DEBUG("test has been run\n");
  while (true) {
    LOG_DEBUG("heartbeat\n");
    delay_s(1);
  }
}

