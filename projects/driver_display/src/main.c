#include <stddef.h>
#include <stdio.h>

#include "delay.h"
#include "driver_display_brightness.h"
#include "driver_display_calibration.h"
#include "driver_display_config.h"
#include "interrupt.h"

int main(void) {
  // Init everything to be used
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);

  // Test calibration
  DriverDisplayCalibrationStorage calibration_storage = { 0 };
  DriverDisplayBrightnessCalibrationData data = { 0 };
  driver_display_calibration_init(driver_display_config_load(), &data, &calibration_storage);
  driver_display_calibration_lower_bound(&calibration_storage);
  driver_display_calibration_upper_bound(&calibration_storage);

  // Temp for debugging
  printf("upper bound: %d \n", calibration_storage.data->max);
  printf("lower bound: %d \n", calibration_storage.data->min);

  // Test brightness module
  // Initialize the brightness module
  DriverDisplayBrightnessStorage storage;
  driver_display_brightness_init(&storage, driver_display_config_load(), &data);

  while (true) {
    // Do stuff
  }
  return 0;
}
