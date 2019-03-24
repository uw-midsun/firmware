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
#include "log.h"
#include "mech_brake.h"
#include "pc_input_event.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

static MechBrakeStorage s_mech_brake;

static StatusCode prv_lsb_to_position(MechBrakeStorage *storage, int16_t reading,
                                      int16_t *position) {
  int16_t input_position = INT16_MAX;
  input_position =
      (EE_PEDAL_OUTPUT_DENOMINATOR * (reading - storage->calibration_data.zero_value)) /
      (storage->calibration_data.hundred_value - storage->calibration_data.zero_value);

  if (input_position < storage->lower_bound || input_position > storage->upper_bound) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  *position = MIN(MAX(0, input_position), EE_PEDAL_OUTPUT_DENOMINATOR);
  return STATUS_CODE_OK;
}

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  MechBrakeStorage *storage = context;
  int16_t position = INT16_MAX;
  StatusCode ret = mech_brake_get_position(storage, &position);

  if (status_ok(ret)) {
    if (position > storage->pressed_threshold_position ||
        (position > storage->unpressed_threshold_position && storage->prev_pressed)) {
      event_raise(INPUT_EVENT_PEDAL_MECHANICAL_BRAKE_PRESSED, (uint16_t)position);
      storage->prev_pressed = true;
    } else {
      event_raise(INPUT_EVENT_PEDAL_MECHANICAL_BRAKE_RELEASED, (uint16_t)position);
      storage->prev_pressed = false;
    }
  }

  LOG_DEBUG()

  pedal_output_update(pedal_output_global(), PEDAL_OUTPUT_SOURCE_MECH_BRAKE, position);
}

StatusCode mech_brake_init(MechBrakeStorage *storage, const MechBrakeSettings *settings,
                           const MechBrakeCalibrationData *data) {
  if (storage == NULL || data == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  storage->calibration_data = *data;
  storage->channel = settings->channel;
  storage->ads1015 = settings->ads1015;

  // Since the minimum value of the position is expected to be 0, the lower bound is the negative
  // value of the tolerance and upper bound is EE_PEDAL_OUTPUT_DENOMINATOR plus the tolerance.
  storage->lower_bound =
      -1 * settings->bounds_tolerance_percentage * EE_PEDAL_OUTPUT_DENOMINATOR / 100;
  storage->upper_bound = EE_PEDAL_OUTPUT_DENOMINATOR +
                         settings->bounds_tolerance_percentage * EE_PEDAL_OUTPUT_DENOMINATOR / 100;

  storage->pressed_threshold_position =
      settings->brake_pressed_threshold_percentage * EE_PEDAL_OUTPUT_DENOMINATOR / 100;
  storage->unpressed_threshold_position =
      settings->brake_unpressed_threshold_percentage * EE_PEDAL_OUTPUT_DENOMINATOR / 100;

  return ads1015_configure_channel(storage->ads1015, storage->channel, true, prv_callback_channel,
                                   storage);
}

StatusCode mech_brake_get_position(MechBrakeStorage *storage, int16_t *position) {
  if (storage == NULL || position == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  int16_t reading = INT16_MIN;

  status_ok_or_return(ads1015_read_raw(storage->ads1015, storage->channel, &reading));
  return prv_lsb_to_position(storage, reading, position);
}

MechBrakeStorage *mech_brake_global(void) {
  return &s_mech_brake;
}
