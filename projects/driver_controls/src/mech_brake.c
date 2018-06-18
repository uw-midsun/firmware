
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>
#include "adc.h"
#include "ads1015.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "mech_brake.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"
#include "wait.h"


int16_t percentage_converter(MechBrakeStorage* storage) {

  int16_t percentage;

  if (storage->calibration_data->zero_value > storage->calibration_data->hundred_value) {
    percentage =
        ((storage->settings.min_allowed_range * (storage->reading - storage->calibration_data->hundred_value)) /
         (storage->calibration_data->hundred_value - storage->calibration_data->zero_value)) +
        storage->settings.max_allowed_range;
  } else {
    percentage =
        (storage->settings.max_allowed_range * (storage->reading - storage->calibration_data->zero_value)) /
        (storage->calibration_data->hundred_value - storage->calibration_data->zero_value);
  }

  if (percentage < storage->settings.min_allowed_range) {
    percentage = storage->settings.min_allowed_range;
  } else if (percentage > storage->settings.max_allowed_range) {
    percentage = storage->settings.max_allowed_range;
  }

  // add code to see if the percentage is greater than max allowed value
  if (percentage > storage->settings.percentage_threshold) {
    uint16_t percentage_data = (uint16_t)percentage;
    event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage_data);
  } else {
    uint16_t percentage_data = (uint16_t)percentage;
    event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage_data);
  }

  return percentage;
}

StatusCode mech_brake_init(MechBrakeStorage* storage, MechBrakeSettings* settings, MechBrakeCalibrationData* data){

  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;
  storage->calibration_data = data;

  return STATUS_CODE_OK;
}
