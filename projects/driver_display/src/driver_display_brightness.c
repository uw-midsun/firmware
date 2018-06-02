#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"

#include "driver_display_brightness.h"
#include "driver_display_config.h"

void driver_display_brightness_calibration(GPIOAddress adc_address) {
  // Configures the lower and upper bounds of the given photodiode
  // Needed to calculate percentage (current brightness value / max brightness value)
  // The percentage is then used in pwm_set_dc which takes in a percentage value
}

void driver_display_brightness_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  // Read raw value from adc_channel and return
  adc_read_raw(adc_channel, adc_reading);
}

void driver_display_brightness_read(GPIOAddress adc_address) {
  uint16_t reading;
  ADCChannel adc_channel;
  adc_get_channel(adc_address, &adc_channel);
  adc_register_callback(adc_channel, driver_display_brightness_callback, &reading);

  // Convert the raw reading into a percentage of max reading to then be passed into pwm_set_dc to
  // adjust brightness accordingly
  uint16_t percent_reading =
      ((reading - DRIVER_DISPLAY_CONFIG_ADC_MIN) * 100) / DRIVER_DISPLAY_CONFIG_ADC_RANGE;

  // Temporary printing for debug purposes
  printf("adc reading: %d percent reading: %d", reading, percent_reading);
  printf("\n");

  DriverDisplayConfigGpio *screen_info = driver_display_config_get_screen_info();

  // Set the screen brightness through PWM change
  // (uint8_t i=0;i<SIZEOF_ARRAY(screen_info);i++) always throws error so I did this workaround
  for (uint8_t i = 1; i <= SIZEOF_ARRAY(screen_info); i++) {
    pwm_set_dc(screen_info[i - 1].timer, percent_reading);
  }
}
