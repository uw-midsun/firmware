#include <stdint.h>
#include <stdio.h>
#include "ads1015.h"
#include "unity.h"

#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "status.h"

static ADS1015Data data;
GPIOAddress ready_pin = { GPIO_PORT_B, 2 };

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_FAST, .scl = { GPIO_PORT_B, 10 }, .sda = { GPIO_PORT_B, 11 }
};

/*
static void prv_callback(const GPIOAddress *address, void *context) {
  ADS1015Data *data = context;
  printf("converted from channel %d\n", data->current_channel);
}
*/

void setup_test(void) {
  
  
  gpio_init();
  interrupt_init();
  gpio_it_init();

  i2c_init(I2C_PORT_2, &i2c_settings);

  ads1015_init(&data, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  
}

void teardown_test(void) {}

void test_ads_init(void){
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_init(NULL, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, ads1015_init(&data, I2C_PORT_2, ADS1015_ADDRESS_GND, NULL));
  ads1015_init(&data, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
}

/*
void test_ads_config_channel(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    ads1015_configure_channel(&data, ADS1015_CHANNEL_0, true, prv_callback, &data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, 
                    ads1015_configure_channel(&data, ADS1015_CHANNEL_0, true, prv_callback, &data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    ads1015_configure_channel(&data, ADS1015_CHANNEL_0, true, NULL, &data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, 
                    ads1015_configure_channel(&data, NUM_ADS1015_CHANNELS, true, prv_callback, &data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, 
                    ads1015_configure_channel(NULL, NUM_ADS1015_CHANNELS, true, prv_callback, &data));
}
*/