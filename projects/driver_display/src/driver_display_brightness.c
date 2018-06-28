#include <status.h>
#include <stdio.h>
#include <stdlib.h>

#include "driver_display_brightness.h"

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  DriverDisplayBrightnessStorage *storage = (DriverDisplayBrightnessStorage *)context;

  uint16_t reading;
  StatusCode primary_status = adc_read_raw(storage->adc_channel, &reading);

  // Convert the raw reading into a percentage of max reading to then be passed into pwm_set_dc to
  // adjust brightness accordingly
  uint16_t percent_reading;
  // Bounds the percent reading to 0-100
  if (reading < storage->calibration_data->min) {
    percent_reading = 0;
  } else {
    percent_reading = (((reading - storage->calibration_data->min) * 10) /
                       (storage->calibration_data->max - storage->calibration_data->min)) *
                      10;
    if (percent_reading > 100) {
      percent_reading = 100;
    }
  }

  // Temp for debugging
  printf("adc reading: %u \n", percent_reading);

  // Set the screen brightness through PWM change
  // Currently all screens are controlled by single photo sensor (they will have synchronized
  // brightness levels)
  StatusCode secondary_status = pwm_set_dc(storage->settings->timer, percent_reading);
  // Schedule new timer
  StatusCode tertiary_status = soft_timer_start_seconds(storage->settings->update_period_s,
                                                        prv_timer_callback, storage, NULL);

  // Check if anything has failed and set appropriate flag
  if (status_ok(primary_status) && status_ok(secondary_status) && status_ok(tertiary_status)) {
    storage->reading_ok_flag = true;
  } else {
    storage->reading_ok_flag = false;
  }
}

static void prv_adc_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  // Read raw value from adc_channel and return
  adc_read_raw(adc_channel, adc_reading);
}

StatusCode driver_display_brightness_init(
    DriverDisplayBrightnessStorage *storage, const DriverDisplayBrightnessSettings *settings,
    const DriverDisplayBrightnessCalibrationData *calibration_data) {
  if (storage == NULL || calibration_data == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->calibration_data = calibration_data;
  storage->settings = settings;

  GPIOSettings pwm_settings = { .direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_HIGH,
                                .resistor = GPIO_RES_PULLUP,
                                .alt_function = GPIO_ALTFN_4 };

  // Init the pwm for each screen
  for (uint8_t i = 0; i < NUM_DRIVER_DISPLAY_BRIGHTNESS_SCREENS; i++) {
    status_ok_or_return(gpio_init_pin(&settings->screen_address[i], &pwm_settings));
    status_ok_or_return(pwm_init_hz(settings->timer, settings->frequency_hz));
    // set the screen brightness to 50% initially
    status_ok_or_return(pwm_set_dc(settings->timer, 50));
  }

  GPIOSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  // Init the ADC pin (All screens currently controlled by single sensor)
  status_ok_or_return(gpio_init_pin(&settings->adc_address, &adc_settings));
  status_ok_or_return(adc_get_channel(settings->adc_address, &storage->adc_channel));
  status_ok_or_return(adc_set_channel(storage->adc_channel, true));

  uint16_t reading;
  status_ok_or_return(adc_register_callback(storage->adc_channel, prv_adc_callback, &reading));

  return soft_timer_start_seconds(settings->update_period_s, prv_timer_callback, storage, NULL);
}
