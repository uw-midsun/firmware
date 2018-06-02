#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "critical_section.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "soft_timer.h"

#include "driver_display_brightness.h"
#include "driver_display_config.h"

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  adc_init(ADC_MODE_CONTINUOUS);

  // Configure the driver display gpios
  driver_display_config();
  GPIOAddress adc_address = driver_display_config_get_adc();

  // Begin superloop
  while (true) {
    // Read the value (need to add delay so it's not reading every moment)
    // Will probably have a refresh rate of something like every 5s
    driver_display_brightness_read(adc_address);
    // Adding temporary delay of 5s to test
    delay_s(5);
  }

  return 0;
}
