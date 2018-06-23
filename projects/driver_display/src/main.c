#include <stddef.h>
#include <stdio.h>

#include "interrupt.h"

#include "driver_display_brightness.h"
#include "driver_display_calibration.h"
#include "driver_display_config.h"

int main(void) {
  // Init everything to be used
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);

  DriverDisplayBrightnessCalibrationData calibration_data = { 0 };
  driver_display_calibration_init(driver_display_config_load(), &calibration_data);

  /*
  // Populate data for brightness module (temporarily here before calibration is implemented)
  const DriverDisplayBrightnessCalibrationData calibration_data = { .max = 4095, .min = 0 };

  // Initialize the brightness module
  DriverDisplayBrightnessStorage storage;
  driver_display_brightness_init(&storage, driver_display_config_load(), &calibration_data);
  */

  while (true) {
    // Do stuff
  }
  return 0;
}
