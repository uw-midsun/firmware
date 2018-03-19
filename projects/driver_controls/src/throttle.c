// The module operates based on the voltage readings that come from the pedals.
// The pedal occupies two channels of the ADS1015 one of which is chosen to be the "main" channel.
// Everytime ADS1015 finishes a conversion of the pedal input on the main channel,
// prv_flag_update_callback is called to set reading_updated_flag accordingly which determines if
// the readings are up to date. A periodic safety check, prv_raise_event_timer_callback, checks
// for this flag and if ok, it updates the position of the pedal in storage and
// raises the events related to the pedal's position (braking, coasting, and accelerating zones).
// The reading_ok_flag basically holds the actual state of reading_updated_flag when it is reset.
// It also verifies if the second channel is in sync with the main channel to ensure the validity
// of the data.
// If the data turns out to be stale or channels aren't synched, a timeout event will be raised.
// The throttle_get_position function simply reads the position from storage.
#include "throttle.h"
#include <string.h>
#include "event_queue.h"
#include "input_event.h"

// Scales a reading to a measure out of 12 bits based on the given range (min - max).
static uint16_t prv_scale_reading(int16_t reading, int16_t max, int16_t min) {
  return (1 << 12) * (reading - min) / (max - min);
}

static uint16_t prv_get_numerator(int16_t reading, ThrottleZone zone, ThrottleChannel channel,
                                  ThrottleStorage *storage) {
  return prv_scale_reading(
      reading, storage->calibration_data->zone_thresholds[channel][zone][THROTTLE_THRESH_MAX],
      storage->calibration_data->zone_thresholds[channel][zone][THROTTLE_THRESH_MIN]);
}

static bool prv_reading_within_zone(int16_t reading, ThrottleZone zone, ThrottleChannel channel,
                                    ThrottleStorage *storage) {
  return (
      (reading < storage->calibration_data->zone_thresholds[channel][zone][THROTTLE_THRESH_MAX]) &&
      (reading >= storage->calibration_data->zone_thresholds[channel][zone][THROTTLE_THRESH_MIN]));
}

// Returns true if the channels match their supposed relationship.
static bool prv_channels_synced(ThrottleStorage *storage, int16_t reading_main,
                                int16_t reading_secondary) {
  uint16_t numerator_main =
      prv_get_numerator(reading_main, THROTTLE_ZONE_ALL, THROTTLE_CHANNEL_MAIN, storage);

  uint16_t numerator_secondary =
      prv_get_numerator(reading_secondary, THROTTLE_ZONE_ALL, THROTTLE_CHANNEL_SECONDARY, storage);

  return (abs(numerator_main - numerator_secondary)) <=
         storage->calibration_data->channel_readings_tolerance;
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

  InputEvent pedal_events[NUM_THROTTLE_ZONES - 1] = {
    INPUT_EVENT_PEDAL_BRAKE, INPUT_EVENT_PEDAL_COAST, INPUT_EVENT_PEDAL_PRESSED
  };

  if (storage->reading_updated_flag &&
      prv_channels_synced(storage, reading_main, reading_secondary) &&
      prv_reading_within_zone(reading_main, THROTTLE_ZONE_ALL, THROTTLE_CHANNEL_MAIN, storage)) {
    ThrottleZone zone = NUM_THROTTLE_ZONES;
    for (zone = THROTTLE_ZONE_BRAKE; zone < (NUM_THROTTLE_ZONES - 1); zone++) {
      if (prv_reading_within_zone(reading_main, zone, THROTTLE_CHANNEL_MAIN, storage)) {
        break;
      }
    }
    storage->position.zone = zone;
    storage->position.numerator =
        prv_get_numerator(reading_main, zone, THROTTLE_CHANNEL_MAIN, storage);
    storage->reading_ok_flag = true;
    InputEvent event = pedal_events[zone];
    event_raise(event, storage->position.numerator);

  } else {
    storage->reading_ok_flag = false;
    event_raise(INPUT_EVENT_PEDAL_TIMEOUT, 0);
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
