#include "status.h"
#include "stdio.h"
#include "stdlib.h"

#include "driver_display_brightness.h"

static void prv_adjust_brightness(DriverDisplayBrightnessData *brightness_data) {
  uint16_t reading;
  adc_read_raw(brightness_data->sensor_data.channel, &reading);

  // Convert the raw reading into a percentage of max reading to then be passed into pwm_set_dc to
  // adjust brightness accordingly
  uint16_t percent_reading =
      (((reading - brightness_data->sensor_data.min) * 100) /
       (brightness_data->sensor_data.max - brightness_data->sensor_data.min));

  // Temp for debugging
  printf("adc reading: %u \n", brightness_data->sensor_data.percent_reading);

  // Set the screen brightness through PWM change
  // (uint8_t i=0;i<SIZEOF_ARRAY(screen_info);i++) always throws error so I did this workaround
  for (uint8_t i = 0; i < DRIVER_DISPLAY_CONFIG_NUM_SCREENS; i++) {
    // Currently all screens are controlled by single photo sensor (they will have synchronized
    // brightness levels)
    pwm_set_dc(brightness_data->screen_data[i].timer, brightness_data->sensor_data.percent_reading);
  }
}

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  DriverDisplayBrightnessData *brightness_data = (DriverDisplayBrightnessData *)context;
  prv_adjust_brightness(brightness_data);
  // Schedule new timer
  SoftTimerID new_timer_id;
  soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD, prv_timer_callback,
                           (void *)brightness_data, &new_timer_id);
}

static void prv_brightness_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  // Read raw value from adc_channel and return
  adc_read_raw(adc_channel, adc_reading);
}

void driver_display_brightness_init(DriverDisplayBrightnessData *brightness_data) {
  // Init the pwm for each screen
  for (uint8_t i = 0; i < DRIVER_DISPLAY_CONFIG_NUM_SCREENS; i++) {
    gpio_init_pin(&brightness_data->screen_data[i].address,
                  &brightness_data->screen_data[i].settings);
    pwm_init_hz(brightness_data->screen_data[i].timer,
                brightness_data->screen_data[i].frequency_hz);
    pwm_set_dc(brightness_data->screen_data[i].timer,
               brightness_data->sensor_data
                   .percent_reading);  // set the screen brightness to 50% initially
  }

  // Init the ADC pin (All screens currently controlled by single sensor)
  ADCChannel adc_channel;
  gpio_init_pin(&brightness_data->sensor_data.address, &brightness_data->sensor_data.settings);
  adc_get_channel(brightness_data->sensor_data.address, &brightness_data->sensor_data.channel);
  adc_set_channel(brightness_data->sensor_data.channel, true);

  uint16_t reading;
  adc_register_callback(brightness_data->sensor_data.channel, prv_brightness_callback, &reading);

  SoftTimerID timer_id;
  soft_timer_start_seconds(DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD, prv_timer_callback,
                           (void *)brightness_data, &timer_id);
}
