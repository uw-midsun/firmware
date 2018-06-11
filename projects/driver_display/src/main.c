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
#include "flash.h"
#include "crc32.h"

#include "driver_display_brightness.h"
#include "driver_display_config.h"

void timer_callback(SoftTimerID timer_id, void *context){
}

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

  // Configuration mode currenty commented out 
  // Would need to be run but currently no GPIO set to active the config mode 
  //driver_display_brightness_calibration(adc_address); 

static const DriverDisplayConfigGpio screen_info[] = {
  [DRIVER_DISPLAY_SCREEN1] = { .address = { DRIVER_DISPLAY_CONFIG_SCREEN1_PORT, DRIVER_DISPLAY_CONFIG_SCREEN1_PIN },
                               .timer = DRIVER_DISPLAY_CONFIG_SCREEN1_TIMER,
                               .frequency = DRIVER_DISPLAY_CONFIG_SCREEN1_FREQ },
};

GPIOSettings screen_pwm_settings = { .direction = GPIO_DIR_OUT,
                                     .state = GPIO_STATE_HIGH,
                                     .resistor = GPIO_RES_PULLUP,
                                     .alt_function = GPIO_ALTFN_4 };

  gpio_init_pin(&screen_info[0].address, &screen_pwm_settings);
  pwm_init(screen_info[0].timer, 500);  // frequency converted to microseconds
  pwm_set_dc(screen_info[0].timer, 50);       // set the screen brightness to 50% initially

  //Start the soft timer
  /*uint8_t reading;
  driver_display_brightness_init();
  soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD, timer_callback, (void*)&reading, DRIVER_DISPLAY_CONFIG_REFRESH_TIMER);*/

  // Begin superloop
  while (true) {
    // Read the value (need to add delay so it's not reading every moment)
    // Will probably have a refresh rate of something like every 5s
    /*if (soft_timer_remaining_time(DRIVER_DISPLAY_CONFIG_REFRESH_TIMER) == 0 ){
      // When the timer runs out restart it and read the brightness values 
      soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD, timer_callback, (void*)&reading, DRIVER_DISPLAY_CONFIG_REFRESH_TIMER);
      driver_display_brightness_read(adc_address);
    }*/
  }
  return 0;
}
