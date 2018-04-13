#include "ads1015_def.h"
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
static DebouncerInfo s_debouncer_info;
static bool s_continue_flag;
static PersistStorage s_persist;

#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A ADS1015_CHANNEL_0
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B ADS1015_CHANNEL_1
#define TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE 30
#define TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE 10

static void prv_wait_for_button(void) {
  s_continue_flag = false;
  while (!s_continue_flag) {
  }
}

static void prv_continue_calibration(const GPIOAddress *address, void *context) {
  s_continue_flag = true;
}

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
  GPIOAddress calibration_button = {
    .port = 0,  //
    .pin = 0,   //
  };

  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  pedal_calibration_init(&s_storage, &s_ads1015_storage, TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A,
                         TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B);
  debouncer_init_pin(&s_debouncer_info, &calibration_button, prv_continue_calibration, NULL);
}

void test_pedal_calibration(void) {
  prv_wait_for_button();

  pedal_calibration_get_band(&s_storage, PEDAL_CALIBRATION_STATE_FULL_BRAKE);

  prv_wait_for_button();

  pedal_calibration_get_band(&s_storage, PEDAL_CALIBRATION_STATE_FULL_THROTTLE);

  pedal_calibration_calculate(&s_storage, &s_throttle_calibration_data,
                              TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE,
                              TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE);
}

void teardown_test(void) {}
