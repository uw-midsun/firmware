#include <stdbool.h>

#include "unity.h"
#include "log.h"
#include "test_helpers.h"
#include "ads1015.h"
#include "delay.h"

static bool s_callback_ran;
static uint8_t s_callback_runs;

static void prv_callback(ADS1015Channel channel, void *context) {
  s_callback_ran = true;
  s_callback_runs++;
}

static void prv_adc_check_range(ADS1015Channel channel) {
  uint16_t reading;

  for (uint8_t i = 0; i < 12; i++) {
    ads1015_read_raw(channel, &reading);
    TEST_ASSERT_TRUE(0 <= reading && reading < 0x7FF);
  }
}

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
  ads1015_init(I2C_PORT_1, address);

  s_callback_ran = false;
  s_callback_runs = 0;

  ads1015_init(I2C_PORT_1, address);
}

void teardown_test(void) { }

void test_ads1015_read(void) {
  TEST_ASSERT_NOT_OK(ads1015_read_raw(NUM_ADS1015_CHANNELS, NULL));
  prv_adc_check_range(ADS1015_CHANNEL_0);
}

void test_ads1015_callback(void) {
  TEST_ASSERT_NOT_OK(ads1015_register_callback(NUM_ADS1015_CHANNELS, prv_callback, NULL));

  TEST_ASSERT_OK(ads1015_register_callback(ADS1015_CHANNEL_0, prv_callback, NULL));

  while (!s_callback_ran) {
    printf("ran = %d | runs = %d\n", s_callback_ran, s_callback_runs);
  }

  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_TRUE(s_callback_runs > 0);
}
