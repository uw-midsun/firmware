#include <status.h>
#include <stdio.h>
#include <stdlib.h>

#include "driver_display_brightness.h"

static void prv_adjust_brightness(DriverDisplayBrightnessStorage *storage) {
  uint16_t reading;
  adc_read_raw(storage->adc_channel, &reading);

  // Convert the raw reading into a percentage of max reading to then be passed into pwm_set_dc to
  // adjust brightness accordingly
  uint16_t percent_reading = (((reading - storage->calibration_data->min) * 10) /
                              (storage->calibration_data->max - storage->calibration_data->min)) *
                             10;

  // Temp for debugging
  // printf("adc reading: %u \n", percent_reading);

  // Set the screen brightness through PWM change
  for (uint8_t i = 0; i < NUM_DRIVER_DISPLAY_BRIGHTNESS_SCREENS; i++) {
    // Currently all screens are controlled by single photo sensor (they will have synchronized
    // brightness levels)
    pwm_set_dc(storage->settings->timer, percent_reading);
  }
}

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  DriverDisplayBrightnessStorage *storage = (DriverDisplayBrightnessStorage *)context;
  prv_adjust_brightness(storage);

  // Schedule new timer
  soft_timer_start_seconds(storage->settings->update_period_s, prv_timer_callback, (void *)storage,
                           NULL);
}

static void prv_brightness_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  // Read raw value from adc_channel and return
  adc_read_raw(adc_channel, adc_reading);
}

StatusCode driver_display_brightness_init(
    DriverDisplayBrightnessStorage *storage, const DriverDisplayBrightnessSettings *settings,
    const DriverDisplayBrightnessCalibrationData *calibration_data) {
  storage->calibration_data = calibration_data;
  storage->settings = settings;

  GPIOSettings pwm_settings = { .direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_HIGH,
                                .resistor = GPIO_RES_PULLUP,
                                .alt_function = GPIO_ALTFN_4 };

  // Init the pwm for each screen
  for (uint8_t i = 0; i < NUM_DRIVER_DISPLAY_BRIGHTNESS_SCREENS; i++) {
    gpio_init_pin(&settings->screen_address[i], &pwm_settings);
    pwm_init_hz(settings->timer, settings->frequency_hz);
    pwm_set_dc(settings->timer, 50);  // set the screen brightness to 50% initially
  }

  GPIOSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  // Init the ADC pin (All screens currently controlled by single sensor)
  gpio_init_pin(&settings->adc_address, &adc_settings);
  adc_get_channel(settings->adc_address, &storage->adc_channel);
  adc_set_channel(storage->adc_channel, true);

  uint16_t reading;
  adc_register_callback(storage->adc_channel, prv_brightness_callback, &reading);

  return soft_timer_start_seconds(settings->update_period_s, prv_timer_callback, (void *)storage,
                                  NULL);
}
