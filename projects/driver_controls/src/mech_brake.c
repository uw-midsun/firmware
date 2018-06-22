
// #include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>
#include "ads1015.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"

#include "exported_enums.h"
#include "input_event.h"
#include "log.h"
#include "mech_brake.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

// takes in LSB as input and converts it to percentage using a linear relationship
static StatusCode prv_lsb_to_percentage(MechBrakeStorage *storage, int16_t reading,
                                        int16_t *percentage) {
  int16_t input_percent = INT16_MAX;

  input_percent =
      (EE_DRIVE_OUTPUT_DENOMINATOR * (reading - storage->calibration_data->zero_value)) /
      (storage->calibration_data->hundred_value - storage->calibration_data->zero_value);

  int16_t lower_bound = -1 * storage->settings.tolerance * EE_DRIVE_OUTPUT_DENOMINATOR / 100;
  int16_t upper_bound =
      EE_DRIVE_OUTPUT_DENOMINATOR + storage->settings.tolerance * EE_DRIVE_OUTPUT_DENOMINATOR / 100;

  if (input_percent < lower_bound || input_percent > upper_bound) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  if (input_percent < 0) {
    input_percent = 0;
  } else if (input_percent > EE_DRIVE_OUTPUT_DENOMINATOR) {
    input_percent = EE_DRIVE_OUTPUT_DENOMINATOR;
  }

  *percentage = MIN(input_percent, EE_DRIVE_OUTPUT_DENOMINATOR);

  return STATUS_CODE_OK;
}

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  MechBrakeStorage *storage = context;
  int16_t percentage = INT16_MAX;
  StatusCode ret = mech_brake_get_percentage(storage, &percentage);

  if (status_ok(ret)) {
    if (percentage > storage->settings.brake_pressed_threshold) {
      event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, (uint16_t)percentage);
    } else {
      event_raise(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, (uint16_t)percentage);
    }
  }

  if (status_ok(ret)) {
    LOG_DEBUG("C%d: %d\n", channel, percentage);
  } else {
    LOG_DEBUG("invalid\n");
  }
}

StatusCode mech_brake_init(MechBrakeStorage *storage, MechBrakeSettings *settings,
                           MechBrakeCalibrationData *data) {
  if (storage == NULL || data == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;
  storage->calibration_data = data;

  return ads1015_configure_channel(storage->settings.ads1015, storage->settings.channel, true,
                                   prv_callback_channel, storage);
}

StatusCode mech_brake_get_percentage(MechBrakeStorage *storage, int16_t *percentage) {
  int16_t reading = INT16_MAX;
  StatusCode ret = ads1015_read_raw(storage->settings.ads1015, storage->settings.channel, &reading);
  if (!status_ok(ret)) {
    LOG_DEBUG("C%d bad status %d\n", storage->settings.channel, ret);
  }

  return prv_lsb_to_percentage(storage, reading, percentage);
}
