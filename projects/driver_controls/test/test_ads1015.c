#include <stdint.h>
#include <stdio.h>
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "unity.h"

static void prv_callback(Ads1015Channel channel, void *context) {
  uint8_t *callback_called = context;
  (*callback_called) = true;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST, .scl = { GPIO_PORT_B, 10 }, .sda = { GPIO_PORT_B, 11 }
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
}

void teardown_test(void) {}

void test_ads1015_init_invalid_input(void) {
  GPIOAddress ready_pin = { GPIO_PORT_B, 2 };
  Ads1015Storage storage;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(NULL, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, NULL));
}

void test_ads_config_channel_invalid_input(void) {
  Ads1015Storage storage;
  GPIOAddress ready_pin = { GPIO_PORT_B, 2 };

  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
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
      ads1015_configure_channel(NULL, ADS1015_CHANNEL_1, true, prv_callback, &storage));
}

void test_ads1015_read_invalid_input(void) {
  int16_t reading;
  Ads1015Storage storage;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_read_converted(&storage, ADS1015_CHANNEL_0, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_read_raw(&storage, ADS1015_CHANNEL_2, &reading));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(NULL, ADS1015_CHANNEL_2, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, ADS1015_CHANNEL_2, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, NUM_ADS1015_CHANNELS, &reading));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_read_raw(NULL, ADS1015_CHANNEL_2, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_read_raw(&storage, ADS1015_CHANNEL_2, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_raw(&storage, NUM_ADS1015_CHANNELS, &reading));
}

void test_ads1015_channel_callback(void) {
  Ads1015Storage storage;
  GPIOAddress ready_pin = { GPIO_PORT_B, 2 };
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  bool callback_called_0 = false;
  bool callback_called_1 = false;
  bool callback_called_2 = false;
  bool callback_called_3 = false;
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback, &callback_called_0);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, false, prv_callback, &callback_called_1);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, prv_callback, &callback_called_2);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_3, false, prv_callback, &callback_called_3);
  soft_timer_init();
  delay_ms(50);
  TEST_ASSERT_EQUAL(true, callback_called_0);
  TEST_ASSERT_EQUAL(false, callback_called_1);
  TEST_ASSERT_EQUAL(true, callback_called_2);
  TEST_ASSERT_EQUAL(false, callback_called_3);
}
