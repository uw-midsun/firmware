#include "control_stalk.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

#define TEST_CONTROL_STALK_I2C_PORT I2C_PORT_2

static Ads1015Storage s_ads1015;
static GpioExpanderStorage s_expander;
static ControlStalk s_stalk;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                    //
    .scl = { .port = GPIO_PORT_B, .pin = 10 },  //
    .sda = { .port = GPIO_PORT_B, .pin = 11 },  //
  };
  i2c_init(TEST_CONTROL_STALK_I2C_PORT, &i2c_settings);

  GPIOAddress ready_pin = {
    .port = GPIO_PORT_A,  //
    .pin = 9,             //
  };
  ads1015_init(&s_ads1015, TEST_CONTROL_STALK_I2C_PORT, ADS1015_ADDRESS_GND, &ready_pin);

  GPIOAddress int_pin = {
    .port = GPIO_PORT_A,  //
    .pin = 8,             //
  };
  gpio_expander_init(&s_expander, TEST_CONTROL_STALK_I2C_PORT, GPIO_EXPANDER_ADDRESS_0, &int_pin);

  TEST_ASSERT_OK(control_stalk_init(&s_stalk, &s_ads1015, &s_expander));
}

void teardown_test(void) {}

void test_control_stalks_readback(void) {
  LOG_DEBUG("hello\n");
  while (true) {
    wait();
  }
}
