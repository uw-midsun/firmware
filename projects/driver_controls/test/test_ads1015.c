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

// This function is registered as the callback for channels.
static void prv_callback(Ads1015Channel channel, void *context) {
  bool *callback_called = context;
  (*callback_called) = true;
}

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
  i2c_init(I2C_PORT_2, &i2c_settings);
}

void teardown_test(void) {}

void test_ads1015_init_invalid_input(void) {
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  Ads1015Storage storage;
  // Tests a basic use of the function
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));
  // Tests for storage being a null pointer.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(NULL, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));
  // Tests for ready pin being a null pointer.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, NULL));
}

void test_ads_config_channel_invalid_input(void) {
  Ads1015Storage storage;
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };

  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  // Tests a basic use of the function
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true,
                                                              prv_callback, &storage));
  // Tests disabling a channel.
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, false,
                                                              prv_callback, &storage));
  // Tests enabling a channel with no callback (context has no effect).
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, NULL, &storage));
  // Tests for out of bound channel.
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      ads1015_configure_channel(&storage, NUM_ADS1015_CHANNELS, true, prv_callback, &storage));
  // Tests for storage being a null pointer.
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      ads1015_configure_channel(NULL, ADS1015_CHANNEL_1, true, prv_callback, &storage));
}

void test_ads1015_read_invalid_input(void) {
  int16_t reading;
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  Ads1015Storage storage;
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_3, true, NULL, &storage);
  // Tests a correct use of the function.
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_read_converted(&storage, ADS1015_CHANNEL_0, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_read_raw(&storage, ADS1015_CHANNEL_2, &reading));
  // Tests for storage being null.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(NULL, ADS1015_CHANNEL_2, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_read_raw(NULL, ADS1015_CHANNEL_2, &reading));
  // Tests for reading being null.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, ADS1015_CHANNEL_2, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_read_raw(&storage, ADS1015_CHANNEL_2, NULL));

  // Tests for out of bound channel.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, NUM_ADS1015_CHANNELS, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_raw(&storage, NUM_ADS1015_CHANNELS, &reading));
}

// This test checks if the callbacks are called properly for enabled channels.
void test_ads1015_channel_callback(void) {
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  Ads1015Storage storage;
  bool callback_called_0 = false;
  bool callback_called_1 = false;
  bool callback_called_2 = false;
  bool callback_called_3 = false;
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback, &callback_called_0);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, false, prv_callback, &callback_called_1);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, prv_callback, &callback_called_2);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_3, false, prv_callback, &callback_called_3);
  delay_ms(50);
  TEST_ASSERT_EQUAL(true, callback_called_0);
  TEST_ASSERT_EQUAL(false, callback_called_1);
  TEST_ASSERT_EQUAL(true, callback_called_2);
  TEST_ASSERT_EQUAL(false, callback_called_3);
}

// Tests a common order of enabling disabling channels.
void test_ads1015_config_channel_case_0(void) {
  Ads1015Storage storage;
  int16_t reading = ADS1015_READ_UNSUCCESSFUL;
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_3, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, false, NULL, &storage);
  delay_ms(50);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, ADS1015_CHANNEL_0, &reading));
  for (Ads1015Channel channel = ADS1015_CHANNEL_1; channel < NUM_ADS1015_CHANNELS; channel++) {
    ads1015_read_converted(&storage, channel, &reading);
    TEST_ASSERT_TRUE((reading < (ADS1015_CURRENT_FSR / 2)) && (reading >= 0));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_read_converted(&storage, channel, &reading));
  }
}

// Tests enabling and disabling a channel right after the other.
void test_ads1015_config_channel_case_1(void) {
  Ads1015Storage storage;
  int16_t reading = ADS1015_READ_UNSUCCESSFUL;
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, false, NULL, &storage);
  delay_ms(50);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, ADS1015_CHANNEL_1, &reading));
}

// Tests disabling an already disabled channel and enabling after.
void test_ads1015_config_channel_case_2(void) {
  Ads1015Storage storage;
  int16_t reading = ADS1015_READ_UNSUCCESSFUL;
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, false, NULL, &storage);
  delay_ms(50);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    ads1015_read_converted(&storage, ADS1015_CHANNEL_2, &reading));
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, NULL, &storage);
  delay_ms(50);
  ads1015_read_converted(&storage, ADS1015_CHANNEL_2, &reading);
  TEST_ASSERT_TRUE((reading < (ADS1015_CURRENT_FSR / 2)) && (reading >= 0));
}

// Tests a common case where every channel is enabled.
void test_ads1015_config_channel_case_3(void) {
  Ads1015Storage storage;
  int16_t reading = ADS1015_READ_UNSUCCESSFUL;
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, NULL, &storage);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_3, true, NULL, &storage);
  delay_ms(50);
  for (Ads1015Channel channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    ads1015_read_converted(&storage, channel, &reading);
    TEST_ASSERT_TRUE((reading < (ADS1015_CURRENT_FSR / 2)) && (reading >= 0));
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, ads1015_read_converted(&storage, channel, &reading));
  }
}
