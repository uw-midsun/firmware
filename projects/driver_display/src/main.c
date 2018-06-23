#include <stddef.h>
#include <stdio.h>

#include "interrupt.h"

#include "driver_display_brightness.h"

int main(void) {
  // Init everything to be used
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);

  // Populate data for brightness module (temporarily here before calibration is implemented)
  GPIOAddress screen_data[] = {
    [DRIVER_DISPLAY_BRIGHTNESS_SCREEN1] = { GPIO_PORT_A, 7 }, 
    [DRIVER_DISPLAY_BRIGHTNESS_SCREEN2] = { GPIO_PORT_A, 4 } 
  };

  DriverDisplayBrightnessSettings settings = { 
    .screen_address = screen_data,
    .adc_address = { .port = GPIO_PORT_A, .pin = 0 }, 
    .timer = PWM_TIMER_14,
    .frequency_hz = 30000,
    .update_period_s = 5
  };

  DriverDisplayBrightnessCalibrationData calibration_data = {
    .max = 4095,
    .min = 0
  };

  // Initialize the brightness module
  driver_display_brightness_init(driver_display_brightness_global(), settings, calibration_data);

  while (true) {
    // Do stuff
  }
  return 0;
}
