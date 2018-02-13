#include <stdint.h>
#include <stdio.h>
#include "ads1015.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "unity.h"

static Ads1015Storage s_storage;
static GPIOAddress s_ready_pin = { GPIO_PORT_B, 2 };

static void prv_callback(Ads1015Channel channel, void *context) {
  Ads1015Storage *storage = context;
  LOG_DEBUG("channel %d = channel %d\n", storage->current_channel, channel);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST, .scl = { GPIO_PORT_B, 10 }, .sda = { GPIO_PORT_B, 11 }
  };
  i2c_init(I2C_PORT_2, &i2c_settings);

  ads1015_init(&s_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &s_ready_pin);
}

void teardown_test(void) {}

void test_ads1015_init_invalid_input(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(NULL, I2C_PORT_2, ADS1015_ADDRESS_GND, &s_ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(&s_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, NULL));
}

void test_ads_config_channel_invalid_input(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&s_storage, ADS1015_CHANNEL_0, true,
                                                              prv_callback, &s_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&s_storage, ADS1015_CHANNEL_0, false,
                                                              prv_callback, &s_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&s_storage, ADS1015_CHANNEL_0, true,
                                                              NULL, &s_storage));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      ads1015_configure_channel(&s_storage, NUM_ADS1015_CHANNELS, true, prv_callback, &s_storage));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      ads1015_configure_channel(NULL, ADS1015_CHANNEL_1, true, prv_callback, &s_storage));
}

void test_ads1015_read_invalid_input(void) {
  int16_t reading;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(NULL, ADS1015_CHANNEL_2, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&s_storage, ADS1015_CHANNEL_2, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&s_storage, NUM_ADS1015_CHANNELS, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_read_raw(NULL, ADS1015_CHANNEL_2, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_raw(&s_storage, ADS1015_CHANNEL_2, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_raw(&s_storage, NUM_ADS1015_CHANNELS, &reading));
}
