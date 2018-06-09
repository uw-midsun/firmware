#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "magnetic_brake_event_generator.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"

int16_t percentage_converter(MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings) {
  int16_t percentage;

  if (brake_settings->zero_value > brake_settings->hundred_value) {
    percentage =
        ((brake_settings->min_allowed_range * (data->reading - brake_settings->hundred_value)) /
         (brake_settings->hundred_value - brake_settings->zero_value)) +
        brake_settings->max_allowed_range;
  } else {
    percentage =
        (brake_settings->max_allowed_range * (data->reading - brake_settings->zero_value)) /
        (brake_settings->hundred_value - brake_settings->zero_value);
  }

  if (percentage < brake_settings->min_allowed_range) {
    percentage = brake_settings->min_allowed_range;
  } else if (percentage > brake_settings->max_allowed_range) {
    percentage = brake_settings->max_allowed_range;
  }

  // add code to see if the percentage is greater than max allowed value
  if (percentage > brake_settings->percentage_threshold) {
    uint16_t percentage_data = (uint16_t)percentage;
    event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage_data);
  } else {
    uint16_t percentage_data = (uint16_t)percentage;
    event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage_data);
  }

  return percentage;
}

static void input_values(MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings,
                         Ads1015Channel channel) {
  int16_t first_samples[1000];
  int16_t second_samples[1000];
  int16_t temp1, temp2, temp3, temp4;

  printf("%s\n",
         "Brake sensor is calibrating, Please ensure the brake is not being pressed, wait \n "
         "for response to continue");

  delay_s(3);

  for (int i = 0; i < 1000; i++) {
    // ads1015_read_raw(data->storage, channel, &data->reading);
    first_samples[i] = data->percentage;
  }

  // max in range of samples = temp 1 and 3

  // min in range of samples = temp 2 and 4

  for (int i = 0; i < 1000; i++) {
    int16_t max = 0;
    int16_t min = brake_settings->max_allowed_range;

    temp1 = first_samples[i];
    temp2 = first_samples[i];

    if (temp1 > max) {
      max = temp1;
    }

    if (temp2 < min) {
      min = temp2;
    }
  }

  int16_t average_lowest = (temp1 + temp2) / 2;
  brake_settings->zero_value = average_lowest;
  printf("%s %d\n", "zero value", brake_settings->zero_value);

  printf("%s\n",
         "Initial calibration complete, Please press and hold the brake \n"
         "wait for response to continue");

  delay_s(3);

  for (int i = 0; i < 1000; i++) {
    // ads1015_read_raw(data->storage, channel, &data->reading);
    second_samples[i] = data->percentage;
    // printf("%s %d %d\n","second sample", i, second_samples[i]);
  }

  for (int i = 0; i < 1000; i++) {
    int16_t max = 0;
    int16_t min = brake_settings->max_allowed_range;

    temp3 = second_samples[i];
    temp4 = second_samples[i];

    if (temp3 > max) {
      max = temp1;
    }

    if (temp4 < min) {
      min = temp2;
    }
  }

  int16_t average_highest = (temp3 + temp4) / 2;
  brake_settings->hundred_value = average_highest;
  printf("%s %d\n", "hundred value", brake_settings->hundred_value);

  printf("%s\n", "Final calibration complete.");

  delay_s(3);
}

StatusCode magnetic_brake_event_generator_init(MagneticCalibrationData *data,
                                               MagneticBrakeSettings *brake_settings,
                                               Ads1015Channel channel) {
  input_values(data, brake_settings, channel);

  return STATUS_CODE_OK;
}
