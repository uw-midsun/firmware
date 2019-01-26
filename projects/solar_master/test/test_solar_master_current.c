#include "i2c.h"
#include "test_helpers.h"
#include "unity.h"

#include "solar_master_current.h"

#define TEST_CURRENT_I2C_PORT I2C_PORT_1

static Ads1015Storage s_current_ads1015 = { 0 };
static SolarMasterCurrent s_current_storage;

void setup_test(void) {
  const I2CSettings current_i2c_settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 },
  };
  TEST_ASSERT_OK(i2c_init(TEST_CURRENT_I2C_PORT, &current_i2c_settings));
}

void teardown_test(void) {}

void test_solar_master_current_init(void) {
  GpioAddress current_ready_pin = CURRENT_ADC_READY_PIN;
  TEST_ASSERT_OK(ads1015_init(&s_current_ads1015, TEST_CURRENT_I2C_PORT,
                              SOLAR_MASTER_CURRENT_ADC_ADDR, &current_ready_pin));
  TEST_ASSERT_OK(solar_master_current_init(&s_current_storage, &s_current_ads1015));
}
