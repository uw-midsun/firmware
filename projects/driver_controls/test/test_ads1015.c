#include <stdint.h>
#include <stdio.h>
#include "ads1015.h"
#include "unity.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"


static Ads1015Storage storage;
GPIOAddress ready_pin = { GPIO_PORT_B, 2 };

static I2CSettings i2c_settings = {
  .speed = I2C_SPEED_FAST, .scl = { GPIO_PORT_B, 10 }, .sda = { GPIO_PORT_B, 11 }
};

static void prv_callback(Ads1015Channel channel, void *context) {
  Ads1015Storage *storage = context;
  printf("channel %d = channel %d\n", storage->current_channel, channel);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  i2c_init(I2C_PORT_2, &i2c_settings);

  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
}

void teardown_test(void) {}

void test_ads1015_init_invalid_input(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(NULL, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, NULL));
}

void test_ads_config_channel(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true,
                                                              prv_callback, &storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, false,
                                                              prv_callback, &storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, NULL, &storage));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      ads1015_configure_channel(&storage, NUM_ADS1015_CHANNELS, true, prv_callback, &storage));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      ads1015_configure_channel(NULL, NUM_ADS1015_CHANNELS, true, prv_callback, &storage));
}
