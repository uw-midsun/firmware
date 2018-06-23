#include "driver_display_calibration.h"
#include <stdio.h>

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  volatile bool *calibrating = context;
  *calibrating = false;
}

static void prv_adc_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  // Read raw value from adc_channel and return
  adc_read_raw(adc_channel, adc_reading);
}

static void prv_perform_calibration(ADCChannel adc_channel,
                                    DriverDisplayBrightnessCalibrationData *data) {
  volatile bool calibrating = true;

  // Starting calibration after pins have been initialized and adc is set up
  printf("Starting calibration mode for next %d seconds \n", DRIVER_DISPLAY_CALIBRATION_PERIOD_S);
  soft_timer_start_seconds(DRIVER_DISPLAY_CALIBRATION_PERIOD_S, prv_timer_callback,
                           (void *)&calibrating, NULL);

  uint16_t reading;

  while (calibrating) {
    adc_read_raw(adc_channel, &reading);
    data->max = MAX(data->max, reading);
    data->min = MIN(data->min, reading);
    printf("reading: %d \n", reading);
  }

  printf("max: %d min: %d \n", data->max, data->min);
}

StatusCode driver_display_calibration_init(const DriverDisplayBrightnessSettings *settings,
                                           DriverDisplayBrightnessCalibrationData *data) {
  // Initilizes the calibration module for light sensor
  GPIOSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  // Init the ADC pin (All screens currently controlled by single sensor)
  ADCChannel adc_channel;
  gpio_init_pin(&settings->adc_address, &adc_settings);
  adc_get_channel(settings->adc_address, &adc_channel);
  adc_set_channel(adc_channel, true);

  uint16_t reading;
  adc_register_callback(adc_channel, prv_adc_callback, &reading);

  data->max = reading;
  data->min = reading;

  prv_perform_calibration(adc_channel, data);

  return STATUS_CODE_OK;
}
