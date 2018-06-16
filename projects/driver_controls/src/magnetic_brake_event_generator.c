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

static void prv_callback_channel(Ads1015Channel channel, void *context) {

  //need to make storage struct  that has adsstorage, channel, data and  brake_settings rather than having so many parameters

  MagneticCalibrationData data* = context;
  int16_t reading = 0;
  ads1015_read_raw(data->mech_brake_storage, channel, &reading);

  data->min_reading = MIN(data->min_reading, reading);
  data->max_reading = MAX(data->max_reading, reading);

  int16_t average_val = (data->max_reading + data->min_reading) / 2;

  int16_t percentage = percentage_converter(data, &brake_settings);
  
  printf("%d %d\n", average_val, percentage);

}

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


static void mech_brake_sample(int16_t percentage, int16_t allowed_range, int16_t *value_needed,
                         Ads1015Channel channel, Ads1015Storage* storage, MagneticBrakeSettings *brake_settings, 
                         MagneticCalibrationData* data){

//Disables channel
ads1015_configure_channel(storage,channel, false,
                              NULL, NULL);

brake_settings->percentage_threshold = 500;
brake_settings->zero_value = 513;
brake_settings->hundred_value = 624;
brake_settings->min_allowed_range = 0;
brake_settings->max_allowed_range = (1<<12);

//enables channel
ads1015_configure_channel(storage, channel, true,
                              prv_callback_channel, storage);

}

StatusCode magnetic_brake_calibration(int16_t percentage, int16_t allowed_range, int16_t *value_needed,
                         Ads1015Channel channel) {

  mech_brake_sample(percentage, allowed_range, value_needed, channel);

  return STATUS_CODE_OK;
}
