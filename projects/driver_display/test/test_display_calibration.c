#include "driver_display_calibration.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "persist.h"
#include "unity.h"

static PersistStorage s_persist_storage;
static DriverDisplayCalibrationStorage s_calibration_storage;
static DriverDisplayBrightnessCalibrationData s_calibration_data;
static DriverDisplayBrightnessStorage s_brightness_storage;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);
  crc32_init();
  flash_init();
}

void teardown_test(void) {}

void test_display_brightness_calibration(void) {
  driver_display_calibration_init(driver_display_brightness_config_load(), &s_calibration_data,
                                  &s_calibration_storage);
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_UPPER_BOUND);
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_LOWER_BOUND);

  // TEST_ASSERT_EQUAL(s_calibration_storage.data->max, 0);
}
