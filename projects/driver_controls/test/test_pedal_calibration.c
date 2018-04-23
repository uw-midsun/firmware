#include "ads1015_def.h"
#include "crc32.h"
#include "debouncer.h"
#include "delay.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "pedal_calibration.h"
#include "persist.h"
#include "test_helpers.h"
#include "unity.h"

static PedalCalibrationStorage s_storage;
static Ads1015Storage s_ads1015_storage;
static ThrottleCalibrationData s_throttle_calibration_data;
static PersistStorage s_persist;

#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A ADS1015_CHANNEL_0
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B ADS1015_CHANNEL_1
#define TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE 30
#define TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE 10
#define TEST_PEDAL_CALIBRATION_SAFETY_FACTOR 2


void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  crc32_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                    //
    .scl = { .port = GPIO_PORT_B, .pin = 10 },  //
    .sda = { .port = GPIO_PORT_B, .pin = 11 },  //
  };
  i2c_init(TEST_ADS1015_I2C_PORT, &i2c_settings);
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };

  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
}

void test_pedal_calibration(void) {}

void teardown_test(void) {}
