#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "crc32.h"
#include "gpio.h"
#include "log.h"
#include "misc.h"
#include "persist.h"
#include "soft_timer.h"
#include "status.h"

#include "driver_display_brightness.h"
#include "driver_display_config.h"

static PersistStorage s_persist;
static DriverDisplayBrightnessData data;

void driver_display_brightness_timer_callback(SoftTimerID timer_id, void *context) {
  flash_erase(DRIVER_DISPLAY_CONFIG_PERSIST_PAGE);
  persist_init(&s_persist, DRIVER_DISPLAY_CONFIG_PERSIST_PAGE, &data, sizeof(data));
}

void driver_display_brightness_calibration(GPIOAddress adc_address) {
  // Configures the lower and upper bounds of the given photodiode
  // Needed to calculate percentage (current brightness value / max brightness value)
  // The percentage is then used in pwm_set_dc which takes in a percentage value
  ADCChannel adc_channel;
  adc_get_channel(adc_address, &adc_channel);
  LOG_DEBUG("starting config mode for %d seconds", DRIVER_DISPLAY_CONFIG_CALIBRATION_TIME);

  uint16_t reading;
  soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD,
                           driver_display_brightness_timer_callback, (void *)&reading,
                           DRIVER_DISPLAY_CONFIG_REFRESH_TIMER);
  data.persistStorage = &s_persist;

  // Loop for certain period of time using soft timer
  while (soft_timer_remaining_time(0) != 0) {
    adc_register_callback(adc_channel, driver_display_brightness_callback, &reading);
    data.max_brightness = MAX(data.max_brightness, reading);
    data.min_brightness = MIN(data.max_brightness, reading);
    data.range_brightness = data.max_brightness - data.min_brightness;
  }
}

void driver_display_brightness_init(void) {
  DriverDisplayBrightnessData new_data = { 0 };
  persist_init(&s_persist, DRIVER_DISPLAY_CONFIG_PERSIST_PAGE, &new_data, sizeof(new_data));
  data = new_data;
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
      (((reading - data.min_brightness) * 100) / data.range_brightness) * 100;

  DriverDisplayConfigGpio *screen_info = driver_display_config_get_screen_info();

  // Set the screen brightness through PWM change
  // (uint8_t i=0;i<SIZEOF_ARRAY(screen_info);i++) always throws error so I did this workaround
  for (uint8_t i = 1; i <= SIZEOF_ARRAY(screen_info); i++) {
    // Currently all screens are controlled by single photo sensor (they will have synchronized
    // brightness levels)
    pwm_set_dc(screen_info[i - 1].timer, percent_reading);
  }
}
