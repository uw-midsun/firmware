// The module operates based on the voltage readings that come from the pedals.
// The pedal occupies two channels of the ADS1015. The channel with a higher resolution is chosen
// to be the "main" channel.
// Everytime ADS1015 finishes a conversion of the pedal input on the main channel,
// prv_flag_update_callback is called to set reading_updated_flag accordingly which determines if
// the readings are up to date. A periodic safety check, prv_raise_event_timer_callback, checks
// for this flag and if ok, it updates the position of the pedal in storage and
// raises the events related to the pedal's position (braking, coasting, and accelerating zones).
// The reading_ok_flag basically holds the actual state of reading_updated_flag as it resets.
// In this module, it is assumed that channels hold a linear relationship with respect to the
// pedal's position. This is used to verify if the readings are valid and "real" i.e if channels
// are synced. If the data turns out to be stale or channels aren't synced, a timeout event will
// be raised. The throttle_get_position function simply reads the position from storage.
#include "throttle.h"
#include <string.h>
#include "event_queue.h"
#include "input_event.h"

// Scales a reading to a measure out of 12 bits based on the given range(min to max).
static uint16_t prv_scale_reading(int16_t reading, int16_t max, int16_t min) {
  return (1 << 12) * (reading - min) / (max - min);
}

// Given the reading(y), finds the corresponding pedal's position(x) according to line of best fit.
// X-axis -> (0 to 2^12), Y-axis -> voltage.
static uint16_t prv_get_numerator(int16_t reading, ThrottleChannel channel,
                                  ThrottleStorage *storage) {
  int16_t max = storage->calibration_data->line_of_best_fit[channel][THROTTLE_THRESH_MAX];
  int16_t min = storage->calibration_data->line_of_best_fit[channel][THROTTLE_THRESH_MIN];
  if (reading < min) {
    return 0;
  } else if (reading > max) {
    return (1 << 12);
  }
  return prv_scale_reading(reading, max, min);
}

// Given a reading from main channel and a zone, finds how far within that zone the pedal is pushed.
// The scale goes from 0(0%) to 2^12(100%). For ex. pedal is at 2^11(50%) in brake zone.
static uint16_t prv_get_numerator_zone(int16_t reading_main, ThrottleZone zone,
                                       ThrottleStorage *storage) {
  return prv_scale_reading(
      reading_main, storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MAX],
      storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MIN]);
}

// Given a reading from main channel and a zone, finds if the reading belongs to that zone.
static bool prv_reading_within_zone(int16_t reading_main, ThrottleZone zone,
                                    ThrottleStorage *storage) {
  return (
      (reading_main <=
       storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MAX]) &&
      (reading_main >= storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MIN]));
}

// Returns true if the channel readings follow their linear relationship.
static bool prv_channels_synced(int16_t reading_main, int16_t reading_secondary,
                                ThrottleStorage *storage) {
  int16_t max_main =
      storage->calibration_data->line_of_best_fit[THROTTLE_CHANNEL_MAIN][THROTTLE_THRESH_MAX];
  int16_t min_main =
      storage->calibration_data->line_of_best_fit[THROTTLE_CHANNEL_MAIN][THROTTLE_THRESH_MIN];
  int16_t tolerance_main = storage->calibration_data->tolerance[THROTTLE_CHANNEL_MAIN];

  if ((reading_main < min_main && (min_main - reading_main) > tolerance_main) ||
      (reading_main > max_main && (reading_main - max_main) > tolerance_main)) {
    return false;
  }
  // Gets pedal's position given the reading from main channel on line of best fit for main channel.
  uint16_t numerator_main = prv_get_numerator(reading_main, THROTTLE_CHANNEL_MAIN, storage);

  int16_t max_secondary =
      storage->calibration_data->line_of_best_fit[THROTTLE_CHANNEL_SECONDARY][THROTTLE_THRESH_MAX];
  int16_t min_secondary =
      storage->calibration_data->line_of_best_fit[THROTTLE_CHANNEL_SECONDARY][THROTTLE_THRESH_MIN];
  int16_t tolerance_secondary = storage->calibration_data->tolerance[THROTTLE_CHANNEL_SECONDARY];

  // Refers to y = mx + b for line of best fit of secondary channel.
  // m(slope) = (max - min) / 2^12. x(pedal's position) = numerator_main.
  // b(y-intercept) = min.
  int16_t mx = (max_secondary - min_secondary) * numerator_main / (1 << 12);
  int16_t b = min_secondary;

  // So this gives expected reading on the secondary channel, by feeding pedal's position obtained
  // from main channel to the equation for seconday channel's line of best fit.
  int16_t expected_reading_secondary = mx + b;

  // Checks if the seconday channel reading is within given bounds around the expected reading.
  return abs(expected_reading_secondary - reading_secondary) <= tolerance_secondary;
}

// This callback is called whenever a conversion is done. It sets the flags
// so that we know conversions are happening.
static void prv_flag_update_callback(Ads1015Channel channel, void *context) {
  ThrottleStorage *storage = context;
  storage->reading_updated_flag = true;
}

// The periodic callback which checks if readings are up to date and channels are in sync.
// If they are, the event corresponding to the pedals position is raised.
// If not, a pedal timout event is raised.
static void prv_raise_event_timer_callback(SoftTimerID timer_id, void *context) {
  ThrottleStorage *storage = context;
  int16_t reading_main = INT16_MIN;
  int16_t reading_secondary = INT16_MIN;

  ads1015_read_raw(storage->pedal_ads1015_storage, storage->channel_main, &reading_main);
  ads1015_read_raw(storage->pedal_ads1015_storage, storage->channel_secondary, &reading_secondary);

  InputEvent pedal_events[NUM_THROTTLE_ZONES] = { INPUT_EVENT_PEDAL_BRAKE, INPUT_EVENT_PEDAL_COAST,
                                                  INPUT_EVENT_PEDAL_PRESSED };
  bool fault = true;
  if (storage->reading_updated_flag &&
      prv_channels_synced(reading_main, reading_secondary, storage)) {
    for (ThrottleZone zone = THROTTLE_ZONE_BRAKE; zone < NUM_THROTTLE_ZONES; zone++) {
      if (prv_reading_within_zone(reading_main, zone, storage)) {
        fault = false;
        storage->position.zone = zone;
        storage->position.numerator = prv_get_numerator_zone(reading_main, zone, storage);
        storage->reading_ok_flag = true;
        event_raise(pedal_events[zone], storage->position.numerator);
        break;
      }
    }
  }
  if (fault) {
    storage->reading_ok_flag = false;
    storage->position.zone = NUM_THROTTLE_ZONES;
    event_raise(INPUT_EVENT_PEDAL_FAULT, 0);
  }

  storage->reading_updated_flag = false;
  soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, context,
                          &storage->raise_event_timer_id);
}

// Initializes the throttle by configuring the ADS1015 channels and
// setting the periodic safety check callback.
StatusCode throttle_init(ThrottleStorage *storage, ThrottleCalibrationData *calibration_data,
                         Ads1015Storage *pedal_ads1015_storage, Ads1015Channel channel_main,
                         Ads1015Channel channel_secondary) {
  if (storage == NULL || calibration_data == NULL || pedal_ads1015_storage == NULL ||
      channel_main >= NUM_ADS1015_CHANNELS || channel_secondary >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;

  // The callback for updating flags is only set on the main channel.
  // Verifying later if the second channel is in sync with the main channel is sufficient.
  status_ok_or_return(ads1015_configure_channel(pedal_ads1015_storage, channel_main, true,
                                                prv_flag_update_callback, storage));
  status_ok_or_return(
      ads1015_configure_channel(pedal_ads1015_storage, channel_secondary, true, NULL, NULL));

  storage->pedal_ads1015_storage = pedal_ads1015_storage;
  storage->channel_main = channel_main;
  storage->channel_secondary = channel_secondary;
  return soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, storage,
                                 &storage->raise_event_timer_id);
}

// Gets the current position of the pedal (writes to position).
StatusCode throttle_get_position(ThrottleStorage *storage, ThrottlePosition *position) {
  if (storage == NULL || position == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!storage->reading_ok_flag) {
    return status_code(STATUS_CODE_TIMEOUT);
  }
  position->zone = storage->position.zone;
  position->numerator = storage->position.numerator;
  return STATUS_CODE_OK;
}
