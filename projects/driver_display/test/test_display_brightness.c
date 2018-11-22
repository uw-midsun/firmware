#include "delay.h"
#include "driver_display_brightness_config.h"
#include "driver_display_calibration.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static DriverDisplayCalibrationStorage s_calibration_storage;
static DriverDisplayBrightnessCalibrationData s_calibration_data;
static DriverDisplayBrightnessStorage s_brightness_storage;
static uint16_t s_mock_reading;

#define CALIBRATION_MAX 1000
#define CALIBRATION_MIN 10
#define ADC_MOCK_DELAY 100

StatusCode TEST_MOCK(adc_read_raw) (AdcChannel adc_channel, uint16_t* reading) {
  LOG_DEBUG("Mock adc_read_raw\n");
  *reading = s_mock_reading;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_register_callback) (AdcChannel channel, AdcCallback callback, void* context) {
  LOG_DEBUG("Mock adc_register_callback\n");
  callback(channel, context);
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_get_channel) (GpioAddress address, AdcChannel *adc_channel) {
  *adc_channel = address.pin;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_set_channel) (AdcChannel adc_channel, bool new_state) {
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(pwm_set_dc) (PwmTimer timer, uint16_t dc) {
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(pwm_init) (PwmTimer timer, uint16_t period_us) {
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  s_calibration_data.max = CALIBRATION_MAX;
  s_calibration_data.min = CALIBRATION_MIN;

  driver_display_brightness_init(&s_brightness_storage,
                                driver_display_brightness_config_load(),
                                &s_calibration_data);
}

void teardown_test(void) {}

void test_invalid_args(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    driver_display_brightness_init(&s_brightness_storage,
                                                  driver_display_brightness_config_load(),
                                                  &s_calibration_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    driver_display_brightness_init(NULL,
                                                  driver_display_brightness_config_load(),
                                                  &s_calibration_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    driver_display_brightness_init(&s_brightness_storage,
                                                  NULL,
                                                  &s_calibration_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    driver_display_brightness_init(&s_brightness_storage,
                                                  driver_display_brightness_config_load(),
                                                  NULL));
}

void test_display_brightness_readings(void) {
  s_mock_reading = 520;
  delay_ms(DRIVER_DISPLAY_CONFIG_UPDATE_PERIOD_S * 1000);
  TEST_ASSERT_EQUAL(50, s_brightness_storage.previous_percent_reading[0]);
}
