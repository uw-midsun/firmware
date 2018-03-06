// The module operates based on the voltage readings that come from the pedals.
// The pedal occupies two channels of the ADS1015 one of which is chosen to be the "main" channel.
// Everytime ADS1015 finishes a conversion of the pedal input on the main channel,
// prv_flag_update_callback is called to set two flags accordingly which determine if
// the readings are up to date. A periodic safety check, prv_raise_event_timer_callback, checks
// for these flags and if ok, it updates the position of the pedal in throttle_storage and
// raises the events related to the pedal's position (braking, coasting, and accelerating zones).
// It also verifies if the second channel is in synch with the main channel.
// If the data turns out to be stale or channels aren't synched, a timeout event will be raised.
// The throttle_get_position function simply reads the position from throttle_storage.
#include "throttle.h"
#include <string.h>
#include "event_queue.h"
#include "input_event.h"

// The time period between every update of the pedal readings.
#define THROTTLE_UPDATE_PERIOD_MS 50

// Scales a reading to a measure out of 12 bits based on the given range.
#define THROTTLE_SCALE_READING_TO_12_BITS(reading, upper_bound, lower_bound) \
  ((1 << 12) * ((reading) - (lower_bound)) / ((upper_bound) - (lower_bound)))

// The tolerance for verifying channel readings match their supposed relationship.
// This tolerance is defined for the scaled-to-12bits measures.
#define THROTTLE_CHANNEL_SCALED_READINGS_TOLERANCE 10

// Returns true if the channels match their supposed relationship.
static bool prv_channels_synched(ThrottleStorage *throttle_storage, int16_t reading_main,
                                 int16_t reading_secondary) {
  if (throttle_storage == NULL) {
    return false;
  }
  int16_t fraction_main = THROTTLE_SCALE_READING_TO_12_BITS(
      reading_main, throttle_storage->calibration_data->main_accel_thresh,
      throttle_storage->calibration_data->main_bottom_thresh);

  int16_t fraction_secondary = THROTTLE_SCALE_READING_TO_12_BITS(
      reading_secondary, throttle_storage->calibration_data->secondary_accel_thresh,
      throttle_storage->calibration_data->secondary_bottom_thresh);

  return (abs(fraction_main - fraction_secondary)) < THROTTLE_CHANNEL_SCALED_READINGS_TOLERANCE;
}

// This callback is called whenever a conversion is done. It sets the flags
// so that we know conversions are happening.
static void prv_flag_update_callback(Ads1015Channel channel, void *context) {
  if (context == NULL) {
    return;
  }
  ThrottleStorage *throttle_storage = context;
  throttle_storage->reading_updated_flag = true;
  throttle_storage->reading_ok_flag = true;
}

// The periodic callback which checks if readings are up to date and channels are in synch.
// If they are, the event corresponding to the pedals position is raised.
// If not, a pedal timout event is raised.
static void prv_raise_event_timer_callback(SoftTimerID timer_id, void *context) {
  if (context == NULL) {
    return;
  }
  ThrottleStorage *throttle_storage = context;

  int16_t reading_main;
  int16_t reading_secondary;
  StatusCode code;
  code = ads1015_read_raw(throttle_storage->pedal_ads1015_storage, throttle_storage->channel_main,
                          &reading_main);
  if (!status_ok(code)) {
    event_raise(INPUT_EVENT_PEDAL_TIMEOUT, 0);
    return;
  }
  code = ads1015_read_raw(throttle_storage->pedal_ads1015_storage,
                          throttle_storage->channel_secondary, &reading_secondary);
  if (!status_ok(code)) {
    event_raise(INPUT_EVENT_PEDAL_TIMEOUT, 0);
    return;
  }

  if (throttle_storage->reading_updated_flag &&
      prv_channels_synched(throttle_storage, reading_main, reading_secondary)) {
    if (reading_main < throttle_storage->calibration_data->main_brake_thresh) {
      // Brake zone.
      throttle_storage->position.zone = THROTTLE_ZONE_BRAKE;
      throttle_storage->position.fraction = THROTTLE_SCALE_READING_TO_12_BITS(
          reading_main, throttle_storage->calibration_data->main_brake_thresh,
          throttle_storage->calibration_data->main_bottom_thresh);
      event_raise(INPUT_EVENT_PEDAL_BRAKE, throttle_storage->position.fraction);

    } else if (reading_main < throttle_storage->calibration_data->main_coast_thresh) {
      // Coast zone.
      throttle_storage->position.zone = THROTTLE_ZONE_COAST;
      throttle_storage->position.fraction = THROTTLE_SCALE_READING_TO_12_BITS(
          reading_main, throttle_storage->calibration_data->main_coast_thresh,
          throttle_storage->calibration_data->main_brake_thresh);
      event_raise(INPUT_EVENT_PEDAL_COAST, throttle_storage->position.fraction);

    } else {
      // Acceleration zone.
      throttle_storage->position.zone = THROTTLE_ZONE_ACCEL;
      throttle_storage->position.fraction = THROTTLE_SCALE_READING_TO_12_BITS(
          reading_main, throttle_storage->calibration_data->main_accel_thresh,
          throttle_storage->calibration_data->main_coast_thresh);
      event_raise(INPUT_EVENT_PEDAL_PRESSED, throttle_storage->position.fraction);
    }

  } else {
    throttle_storage->reading_ok_flag = false;
    event_raise(INPUT_EVENT_PEDAL_TIMEOUT, 0);
  }

  throttle_storage->reading_updated_flag = false;
  soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, context,
                          &throttle_storage->raise_event_timer_id);
}

// Initializes the throttle by configuring the ADS1015 channels and
// setting the periodic safety check callback.
StatusCode throttle_init(ThrottleStorage *throttle_storage, Ads1015Storage *pedal_ads1015_storage,
                         Ads1015Channel channel_main, Ads1015Channel channel_secondary) {
  if (throttle_storage == NULL || pedal_ads1015_storage == NULL ||
      channel_main >= NUM_ADS1015_CHANNELS || channel_secondary >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(throttle_storage, 0, sizeof(*throttle_storage));
  // The callback for updating flags is only set on the main channel.
  // Verifying later if the second channel is in synch with the main channel is sufficient.
  status_ok_or_return(ads1015_configure_channel(pedal_ads1015_storage, channel_main, true,
                                                prv_flag_update_callback, throttle_storage));
  status_ok_or_return(
      ads1015_configure_channel(pedal_ads1015_storage, channel_secondary, true, NULL, NULL));

  throttle_storage->pedal_ads1015_storage = pedal_ads1015_storage;
  throttle_storage->channel_main = channel_main;
  throttle_storage->channel_secondary = channel_secondary;
  return soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback,
                                 throttle_storage, &throttle_storage->raise_event_timer_id);
}

// Gets the current position of the pedal (writes to position).
StatusCode throttle_get_position(ThrottleStorage *throttle_storage, ThrottlePosition *position) {
  if (throttle_storage == NULL || position == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (throttle_storage->reading_ok_flag) {
    position->zone = throttle_storage->position.zone;
    position->fraction = throttle_storage->position.fraction;

    return STATUS_CODE_OK;
  }
  return status_code(STATUS_CODE_TIMEOUT);
}

StatusCode throttle_set_calibration_data(ThrottleStorage *throttle_storage,
                                         ThrottleCalibrationData *calibration_data) {
  if (throttle_storage == NULL || calibration_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  throttle_storage->calibration_data = calibration_data;
  return STATUS_CODE_OK;
}
