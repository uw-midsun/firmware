#include <stdio.h>

#include "delay.h"
#include "driver_display_calibration.h"

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  volatile bool *calibrating = context;
  *calibrating = false;
}

StatusCode driver_display_calibration_init(const DriverDisplayBrightnessSettings *settings,
                                           DriverDisplayBrightnessCalibrationData *data,
                                           DriverDisplayCalibrationStorage *storage) {
  if (storage == NULL || data == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->data = data;

  // Initilizes the calibration module for light sensor
  GPIOSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  // Init the ADC pin (All screens currently controlled by single sensor)
  ADCChannel adc_channel;
  status_ok_or_return(gpio_init_pin(&settings->adc_address, &adc_settings));
  status_ok_or_return(adc_get_channel(settings->adc_address, &adc_channel));
  status_ok_or_return(adc_set_channel(adc_channel, true));

  storage->adc_channel = adc_channel;

  return STATUS_CODE_OK;
}

StatusCode driver_display_calibration_upper_bound(DriverDisplayCalibrationStorage *storage) {
  volatile bool calibrating = true;

  // Starting calibration after pins have been initialized and adc is set up
  printf("Starting upper bound calibration mode for next %d seconds \n",
         DRIVER_DISPLAY_CALIBRATION_PERIOD_S);
  status_ok_or_return(soft_timer_start_seconds(DRIVER_DISPLAY_CALIBRATION_PERIOD_S,
                                               prv_timer_callback, (void *)&calibrating, NULL));

  uint64_t sum = 0;
  uint16_t count = 0;

  while (calibrating) {
    uint16_t reading;
    adc_read_raw(storage->adc_channel, &reading);
    sum += reading;
    count++;
    delay_ms(10);  // Delay to prevent overflow
  }

  // Average out the values to obtain upper threshold
  storage->data->max = sum / count;
  return STATUS_CODE_OK;
}

StatusCode driver_display_calibration_lower_bound(DriverDisplayCalibrationStorage *storage) {
  volatile bool calibrating = true;

  // Starting calibration after pins have been initialized and adc is set up
  printf("Starting lower bound calibration mode for next %d seconds \n",
         DRIVER_DISPLAY_CALIBRATION_PERIOD_S);
  status_ok_or_return(soft_timer_start_seconds(DRIVER_DISPLAY_CALIBRATION_PERIOD_S,
                                               prv_timer_callback, (void *)&calibrating, NULL));

  uint64_t sum = 0;
  uint16_t count = 0;

  while (calibrating) {
    uint16_t reading;
    adc_read_raw(storage->adc_channel, &reading);
    sum += reading;
    count++;
    delay_ms(10);  // Delay to prevent overflow
  }

  // Average out the values to obtain lower threshold
  storage->data->min = sum / count;
  return STATUS_CODE_OK;
}
