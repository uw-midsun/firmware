#include "driver_display_brightness_config.h"
#include "driver_display_calibration.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"
#include "test_helpers.h"
#include "unity.h"

static DriverDisplayCalibrationStorage s_calibration_storage;
static DriverDisplayBrightnessCalibrationData s_calibration_data;
static DriverDisplayBrightnessStorage s_brightness_storage;
static uint16_t s_mock_reading;

#define CALIBRATION_MAX 1000
#define CALIBRATION_MIN 10

StatusCode TEST_MOCK(adc_read_raw) (AdcChannel adc_channel, uint16_t* reading) {
  if (reading) *reading = s_mock_reading;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_register_callback) (AdcChannel channel, AdcCallback callback, void* context) {
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

StatusCode TEST_MOCK(soft_timer_start) (uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerId *timer_id) {
  callback(0, context);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
}

void teardown_test(void) {}

void test_display_brightness_calibration(void) {
  driver_display_calibration_init(driver_display_brightness_config_load(), &s_calibration_data,
                                  &s_calibration_storage);
  s_mock_reading = CALIBRATION_MAX;
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_UPPER_BOUND);
  s_mock_reading = CALIBRATION_MIN;
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_LOWER_BOUND);

  TEST_ASSERT_EQUAL(CALIBRATION_MAX, s_calibration_storage.data->max);
  TEST_ASSERT_EQUAL(CALIBRATION_MIN, s_calibration_storage.data->min);
}
