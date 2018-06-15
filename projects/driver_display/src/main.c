#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "crc32.h"
#include "delay.h"
#include "flash.h"
#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "soft_timer.h"

#include "driver_display_brightness.h"
#include "driver_display_config.h"

void timer_callback(SoftTimerID timer_id, void *context) {}

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();
  flash_init();
  crc32_init();

  adc_init(ADC_MODE_CONTINUOUS);

  // Configure the driver display gpios
  driver_display_config();
  GPIOAddress adc_address = driver_display_config_get_adc();

  // CONFIG MODE CURRENTLY COMMENTED OUT (uncomment to config the max and min values of the
  // photodiode) Would need to be run but currently no GPIO set to active the config mode
  // driver_display_brightness_calibration(adc_address);

  // Start the soft timer
  uint8_t reading;
  driver_display_brightness_init();
  soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD, timer_callback, (void *)&reading,
                           DRIVER_DISPLAY_CONFIG_REFRESH_TIMER);

  // Begin superloop
  while (true) {
    // Read the value (need to add delay so it's not reading every moment)
    // Will probably have a refresh rate of something like every 5s
    if (soft_timer_remaining_time(DRIVER_DISPLAY_CONFIG_REFRESH_TIMER) == 0) {
      // When the timer runs out restart it and read the brightness values
      soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD, timer_callback,
                               (void *)&reading, DRIVER_DISPLAY_CONFIG_REFRESH_TIMER);
      driver_display_brightness_read(adc_address);
    }
  }
  return 0;
}
