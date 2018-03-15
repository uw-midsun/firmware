// The module operates based on the voltage readings that come from the pedals.
// The pedal occupies two channels of the ADS1015 one of which is chosen to be the "main" channel.
// Everytime ADS1015 finishes a conversion of the pedal input on the main channel,
// prv_flag_update_callback is called to set two flags accordingly which determine if
// the readings are up to date. A periodic safety check, prv_raise_event_timer_callback, checks
// for these flags and if ok, it updates the position of the pedal in throttle_storage and
// raises the events related to the pedal's position (braking, coasting, and accelerating zones).
// It also verifies if the second channel is in sync with the main channel.
// If the data turns out to be stale or channels aren't synched, a timeout event will be raised.
// The throttle_get_position function simply reads the position from throttle_storage.
#include "throttle.h"
#include <string.h>
#include "event_queue.h"
#include "input_event.h"
#include "ads1015_def.h"

// Scales a reading to a measure out of 12 bits based on the given range (min - max).
static uint16_t prv_scale_reading(int16_t reading, int16_t max, int16_t min) {
  return (1 << 12) * (reading - min) / (max - min);
}

static uint16_t prv_get_numerator(int16_t reading_main, ThrottleZone zone, ThrottleStorage *throttle_storage) {
  return prv_scale_reading(reading_main,
                    throttle_storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MAX],
                    throttle_storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MIN]);
}

// The tolerance for verifying channel readings match their supposed relationship.
// This tolerance is defined for the scaled-to-12bits measures.
#define THROTTLE_CHANNEL_SCALED_READINGS_TOLERANCE 10

static bool prv_reading_within_zone(int16_t reading_main, ThrottleZone zone,
                                    ThrottleStorage *throttle_storage) {
  return (
      (reading_main < throttle_storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MAX]) &&
      (reading_main > throttle_storage->calibration_data->zone_thresholds_main[zone][THROTTLE_THRESH_MIN]));
}

// Returns true if the channels match their supposed relationship.
static bool prv_channels_synced(ThrottleStorage *throttle_storage, int16_t reading_main,
                                int16_t reading_secondary) {

  int16_t upper_bound_main = throttle_storage->calibration_data
                                 ->zone_thresholds_main[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX];
  int16_t lower_bound_main = throttle_storage->calibration_data
                                 ->zone_thresholds_main[THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MIN];
  int16_t upper_bound_secondary =
      throttle_storage->calibration_data
          ->zone_thresholds_secondary[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX];
  int16_t lower_bound_secondary =
      throttle_storage->calibration_data
          ->zone_thresholds_secondary[THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MIN];

  uint16_t numerator_main = prv_scale_reading(reading_main, upper_bound_main, lower_bound_main);

  uint16_t numerator_secondary =
      prv_scale_reading(reading_secondary, upper_bound_secondary, lower_bound_secondary);

  return (abs(numerator_main - numerator_secondary)) < THROTTLE_CHANNEL_SCALED_READINGS_TOLERANCE;
}

// This callback is called whenever a conversion is done. It sets the flags
// so that we know conversions are happening.
static void prv_flag_update_callback(Ads1015Channel channel, void *context) {
  if (context == NULL) {
    return;
  }
  ThrottleStorage *throttle_storage = context;
  throttle_storage->reading_updated_flag = true;
}


// The periodic callback which checks if readings are up to date and channels are in sync.
// If they are, the event corresponding to the pedals position is raised.
// If not, a pedal timout event is raised.
static void prv_raise_event_timer_callback(SoftTimerID timer_id, void *context) {
  ThrottleStorage *throttle_storage = context;
  int16_t reading_main = ADS1015_READ_UNSUCCESSFUL;
  int16_t reading_secondary = ADS1015_READ_UNSUCCESSFUL;

  ads1015_read_raw(throttle_storage->pedal_ads1015_storage, throttle_storage->channel_main,
                          &reading_main);

  ads1015_read_raw(throttle_storage->pedal_ads1015_storage,
                          throttle_storage->channel_secondary, &reading_secondary);

  InputEvent event = NUM_INPUT_EVENTS;
  ThrottleZone zone = NUM_THROTTLE_ZONES;

  if (throttle_storage->reading_updated_flag &&
      prv_channels_synced(throttle_storage, reading_main, reading_secondary)) {
    if (prv_reading_within_zone(reading_main, THROTTLE_ZONE_BRAKE, throttle_storage)) {
      // Brake zone.
      zone = THROTTLE_ZONE_BRAKE;
      event = INPUT_EVENT_PEDAL_BRAKE;
    } else if (prv_reading_within_zone(reading_main, THROTTLE_ZONE_COAST, throttle_storage)) {
      // Coast zone.
      zone = THROTTLE_ZONE_COAST;
      event = INPUT_EVENT_PEDAL_COAST;
    } else {
      // Acceleration zone.
      zone = THROTTLE_ZONE_ACCEL;
      event = INPUT_EVENT_PEDAL_PRESSED;
    }

    throttle_storage->position.zone = zone;
    throttle_storage->position.numerator = prv_get_numerator(reading_main, zone, throttle_storage);
    throttle_storage->reading_ok_flag = true;
    event_raise(event, throttle_storage->position.numerator);

  } else {
    throttle_storage->reading_ok_flag = false;
    event_raise(INPUT_EVENT_PEDAL_TIMEOUT, 0);
  }

  throttle_storage->reading_updated_flag = false;
  soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, context,
                          &throttle_storage->raise_event_timer_id);
}

// Sets calibration data of the throttle_storage to calibration_data.
static StatusCode prv_set_calibration_data(ThrottleStorage *throttle_storage,
                                           ThrottleCalibrationData *calibration_data) {
  if (throttle_storage == NULL || calibration_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  throttle_storage->calibration_data = calibration_data;
  return STATUS_CODE_OK;
}

// Initializes the throttle by configuring the ADS1015 channels and
// setting the periodic safety check callback.
StatusCode throttle_init(ThrottleStorage *throttle_storage,
                         ThrottleCalibrationData *calibration_data,
                         Ads1015Storage *pedal_ads1015_storage, Ads1015Channel channel_main,
                         Ads1015Channel channel_secondary) {
  if (throttle_storage == NULL || pedal_ads1015_storage == NULL ||
      channel_main >= NUM_ADS1015_CHANNELS || channel_secondary >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(throttle_storage, 0, sizeof(*throttle_storage));

  status_ok_or_return(prv_set_calibration_data(throttle_storage, calibration_data));

  // The callback for updating flags is only set on the main channel.
  // Verifying later if the second channel is in sync with the main channel is sufficient.
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
  if (!throttle_storage->reading_ok_flag) {
    return status_code(STATUS_CODE_TIMEOUT);
  }
  position->zone = throttle_storage->position.zone;
  position->numerator = throttle_storage->position.numerator;
  return STATUS_CODE_OK;
}
