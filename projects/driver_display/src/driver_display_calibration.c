#include <stdio.h>

#include "delay.h"
#include "driver_display_calibration.h"

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  DriverDisplayCalibrationStorage *storage = context;
  // Continue sampling until the sample size has been reached
  if (storage->sample_count < DRIVER_DISPLAY_CALIBRATION_SAMPLE_SIZE) {
    uint16_t reading;
    StatusCode primary_status = adc_read_raw(storage->adc_channel, &reading);
    storage->sample_sum += reading;
    storage->sample_count++;

    // Start the timer again
    StatusCode secondary_status = soft_timer_start_millis(DRIVER_DISPLAY_CALIBRATION_PERIOD_MS,
                                                          prv_timer_callback, storage, NULL);

    // Check if anything has failed and set appropriate flag
    if (status_ok(primary_status) && status_ok(secondary_status)) {
      storage->reading_ok_flag = true;
    } else {
      storage->reading_ok_flag = false;
    }
  }
}

static void prv_adc_callback(AdcChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  // Read raw value from adc_channel and return
  adc_read_raw(adc_channel, adc_reading);
}

StatusCode driver_display_calibration_init(const DriverDisplayBrightnessSettings *settings,
                                           DriverDisplayBrightnessCalibrationData *data,
                                           DriverDisplayCalibrationStorage *storage) {
  if (storage == NULL || data == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->data = data;

  // Initilizes the calibration module for light sensor
  GpioSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  // Initialize the ADC pin for brightness signal from photodiode
  // Init the ADC pin (All screens currently controlled by single sensor)
  AdcChannel adc_channel;
  status_ok_or_return(gpio_init_pin(&settings->adc_address, &adc_settings));
  status_ok_or_return(adc_get_channel(settings->adc_address, &adc_channel));
  status_ok_or_return(adc_set_channel(adc_channel, true));

  storage->adc_channel = adc_channel;

  adc_register_callback(adc_channel, prv_adc_callback, NULL);

  return soft_timer_start_millis(DRIVER_DISPLAY_CALIBRATION_PERIOD_MS, prv_timer_callback, storage,
                                 NULL);
}

StatusCode driver_display_calibration_bounds(DriverDisplayCalibrationStorage *storage,
                                             DriverDisplayCalibrationBounds bound) {
  if (bound >= NUM_DRIVER_DISPLAY_CALIBRATION_BOUNDS) {
    // Invalid bound to calibrate
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->sample_count = 0;
  storage->sample_sum = 0;
  storage->reading_ok_flag = true;

  // Starting calibration after pins have been initialized and adc is set up
  // Temp for debugging
  printf("Starting bound calibration mode for next %d ms \n",
         DRIVER_DISPLAY_CALIBRATION_PERIOD_MS * DRIVER_DISPLAY_CALIBRATION_SAMPLE_SIZE);

  status_ok_or_return(soft_timer_start_millis(DRIVER_DISPLAY_CALIBRATION_PERIOD_MS,
                                              prv_timer_callback, storage, NULL));

  while ((storage->sample_count) < DRIVER_DISPLAY_CALIBRATION_SAMPLE_SIZE) {
    // wait for process to collect samples
    if (storage->reading_ok_flag == false) {
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
  }

  // Temp for debugging
  printf("Number of samples taken: %d \n", storage->sample_count);

  // Average out the values to obtain threshold
  if (bound == DRIVER_DISPLAY_CALIBRATION_UPPER_BOUND) {
    storage->data->max = storage->sample_sum / DRIVER_DISPLAY_CALIBRATION_SAMPLE_SIZE;
  } else if (bound == DRIVER_DISPLAY_CALIBRATION_LOWER_BOUND) {
    storage->data->min = storage->sample_sum / DRIVER_DISPLAY_CALIBRATION_SAMPLE_SIZE;
  }
  return STATUS_CODE_OK;
}
