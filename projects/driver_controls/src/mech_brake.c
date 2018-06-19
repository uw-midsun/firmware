

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>
#include "ads1015.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"

#include "input_event.h"
#include "log.h"
#include "mech_brake.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

// takes in LSB as input and converts it to percentage using a linear relationship
static int16_t lsb_to_percentage_converter(MechBrakeStorage *storage) {
  int16_t percentage;

  if (storage->calibration_data->zero_value > storage->calibration_data->hundred_value) {
    percentage =
        ((storage->settings.min_allowed_range *
          (storage->reading - storage->calibration_data->hundred_value)) /
         (storage->calibration_data->hundred_value - storage->calibration_data->zero_value)) +
        storage->settings.max_allowed_range;
  } else {
    percentage = (storage->settings.max_allowed_range *
                  (storage->reading - storage->calibration_data->zero_value)) /
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
    event_raise(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, percentage_data);
  }

  return percentage;
}

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  MechBrakeStorage *storage = context;

  ads1015_read_raw(storage->settings.ads1015, channel, &(storage->reading));

  int16_t percentage = lsb_to_percentage_converter(storage);
  int16_t reading = storage->reading;

  LOG_DEBUG("%d %d\n", storage->reading, percentage);

  storage->percentage = percentage;
}

StatusCode mech_brake_init(MechBrakeStorage *storage, MechBrakeSettings *settings,
                           MechBrakeCalibrationData *data) {
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;
  storage->calibration_data = data;

  ads1015_configure_channel(storage->settings.ads1015, storage->settings.channel, true,
                            prv_callback_channel, storage);

  return STATUS_CODE_OK;
}
